#pragma once

#include "hardware/device/Kinematic.hpp"
#include "hardware/device/trajectory.hpp"
#include "hardware/fsm/FSM.hpp"
#include "rclcpp/node.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <numbers>
#include <rclcpp/logging.hpp>
#include <string>

#include "hardware/fsm/FSM_gold_l.hpp"
class Auto_Gold_Mid {
public:
    explicit Auto_Gold_Mid() {
        reset_initial_arm.set_total_step(1000).set_end_point(
            {0, -0.672690, -0.023241, std::numbers::pi, 0.713385, 0});
        lift_mine.set_total_step(500)
            .set_start_point(
                {0.52, 0, 0.1},
                {-std::numbers::pi, -90 * std::numbers::pi / 180, -std::numbers::pi})
            .set_end_point(
                {0.52, 0, 0.32},
                {-std::numbers::pi, -90 * std::numbers::pi / 180, -std::numbers::pi});

        fsm.registerState<Set_initial_State>();
        fsm.registerState<Lift_State>();
        fsm.addTransition<Auto_Gold_Event>(
            Auto_Gold_State::Set_initial, Auto_Gold_Event::Up,
            [this](const Auto_Gold_Event& event, const Auto_Gold_Context& context) {
                if (event == Auto_Gold_Event::Up) {
                    if (reset_initial_arm.get_complete()) {
                        return true;
                    }
                    reset_initial_arm.set_start_point(last_theta_);
                    result = reset_initial_arm.trajectory();
                }
                return false;
            },
            [this](const Auto_Gold_Event& event, Auto_Gold_Context& context) {},
            Auto_Gold_State::Lift);

        fsm.addTransition<Auto_Gold_Event>(
            Auto_Gold_State::Lift, Auto_Gold_Event::Up,
            [this](const Auto_Gold_Event& event, const Auto_Gold_Context& context) {
                if (event == Auto_Gold_Event::Up) {
                      std::array<double,6> result_ = rmcs_core::hardware::device::Kinematic::inverse_kinematic(
                        lift_mine.trajectory());
                        result_[3] = std::numbers::pi;
                        result_[5] = 0;
                        result = result_;
                    reset_initial_arm.reset();
                }
                if (event == Auto_Gold_Event::Down) {
                    if (lift_mine.get_complete()) {
                        reset_initial_arm.set_total_step(500);
                        reset_initial_arm.set_start_point(last_theta_);
                        result = reset_initial_arm.trajectory();
                        if (reset_initial_arm.get_complete()) {
                            reset_initial_arm.reset();
                            lift_mine.reset();
                        }
                    }
                }
                return false;
            },
            [this](const Auto_Gold_Event& event, Auto_Gold_Context& context) {},
            Auto_Gold_State::Set_initial);

        // fsm.addTransition<Auto_Gold_Event>(
        //     Auto_Gold_State::Lift, Auto_Gold_Event::Down,
        //     [this](const Auto_Gold_Event& event, const Auto_Gold_Context& context) {

        // RCLCPP_INFO(logger, "e");

        //         return false;
        //     },
        //     [this](
        //         const Auto_Gold_Event& event,
        //         Auto_Gold_Context& context) {  },
        //     Auto_Gold_State::Set_initial);
        fsm.start(Auto_Gold_State::Set_initial);
    }
    Auto_Gold_State getState() { return fsm.getCurrentState(); }
    void get_current_theta(std::array<double, 6> theta) { last_theta_ = theta; }
    std::array<double, 6> get_result() { return result; }
    bool get_first_trajectory_result() { return reset_initial_arm.get_complete(); }
    void reset_first_trajectory() { reset_initial_arm.reset(); }
    bool get_second_trajectory_result() { return lift_mine.get_complete(); }

    void processEvent(const Auto_Gold_Event& event) { fsm.processEvent(event); }

    void reset() {
        fsm.start(Auto_Gold_State::Set_initial);
        fsm_direction = initial_enter;

        reset_initial_arm.reset();
        lift_mine.reset();
    }
    int fsm_direction = initial_enter;

private:
    Auto_Gold_Context context_;
    FiniteStateMachine<Auto_Gold_State, Auto_Gold_Event, Auto_Gold_Context> fsm{context_};
    std::array<double, 6> last_theta_, result;
    rclcpp::Logger logger = rclcpp::get_logger("Auto_gold");
    rmcs_core::hardware::device::Trajectory<rmcs_core::hardware::device::TrajectoryType::JOINT>
        reset_initial_arm;

    rmcs_core::hardware::device::Trajectory<rmcs_core::hardware::device::TrajectoryType::LINE>
        lift_mine;
};