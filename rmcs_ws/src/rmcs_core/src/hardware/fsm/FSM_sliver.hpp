#pragma once

#include "hardware/device/Kinematic.hpp"
#include "hardware/device/trajectory.hpp"
#include "hardware/fsm/FSM.hpp"
#include "rclcpp/node.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <numbers>
#include <rclcpp/logging.hpp>
#include <string>

enum class Auto_Sliver_State {
    Set_initial,
    Lift_mine,
};
enum class Auto_Sliver_Event { Up, Down };
struct Auto_Sliver_Context {};
class Sliver_Set_initial_State
    : public IState<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context> {
public:
    void enter(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Context& context) override {}
    void exit(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Context& context) override {}
    std::shared_ptr<IState<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>> handleEvent(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Event& event, Auto_Sliver_Context& context) override {

        if (event == Auto_Sliver_Event::Up) {
            return fsm.getState(Auto_Sliver_State::Lift_mine);
        }
        return nullptr;
    }
    Auto_Sliver_State getStateID() const override { return Auto_Sliver_State::Set_initial; }
};

class Sliver_Lift_mine_State
    : public IState<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context> {
public:
    void enter(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Context& context) override {}
    void exit(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Context& context) override {}
    std::shared_ptr<IState<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>> handleEvent(
        FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context>& fsm,
        const Auto_Sliver_Event& event, Auto_Sliver_Context& context) override {

        if (event == Auto_Sliver_Event::Down) {
            return fsm.getState(Auto_Sliver_State::Set_initial);
        }
        return nullptr;
    }
    Auto_Sliver_State getStateID() const override { return Auto_Sliver_State::Lift_mine; }
};

class Auto_Sliver {
public:
    explicit Auto_Sliver() {
        reset_initial_arm.set_total_step(1000).set_end_point(
            {0, -0.672690, -0.023241, std::numbers::pi, 0.713385, 0});
        lift_mine.set_total_step(500)
            .set_start_point(
                {0.52, 0, 0.1},
                {-std::numbers::pi, -90 * std::numbers::pi / 180, -std::numbers::pi})
            .set_end_point(
                {0.52, 0, 0.32},
                {-std::numbers::pi, -90 * std::numbers::pi / 180, -std::numbers::pi});

        fsm.registerState<Sliver_Set_initial_State>();
        fsm.registerState<Sliver_Lift_mine_State>();
        fsm.addTransition<Auto_Sliver_Event>(
            Auto_Sliver_State::Set_initial, Auto_Sliver_Event::Up,
            [this](const Auto_Sliver_Event& event, const Auto_Sliver_Context& context) {
                if (event == Auto_Sliver_Event::Up) {
                    if (reset_initial_arm.get_complete()) {
                        return true;
                    }
                    reset_initial_arm.set_start_point(enter_theta_);
                    result = reset_initial_arm.trajectory();
                }
                return false;
            },
            [this](const Auto_Sliver_Event& event, Auto_Sliver_Context& context) {},
            Auto_Sliver_State::Lift_mine);

        fsm.addTransition<Auto_Sliver_Event>(
            Auto_Sliver_State::Lift_mine, Auto_Sliver_Event::Up,
            [this](const Auto_Sliver_Event& event, const Auto_Sliver_Context& context) {
                if (event == Auto_Sliver_Event::Up) {
                    std::array<double, 6> result_ =
                        rmcs_core::hardware::device::Kinematic::inverse_kinematic(
                            lift_mine.trajectory());
                    result_[3] = std::numbers::pi;
                    result_[5] = 0;
                    result     = result_;
                    reset_initial_arm.reset();
                }
                if (event == Auto_Sliver_Event::Down) {
                    if (lift_mine.get_complete()) {
                        reset_initial_arm.set_total_step(500);
                        reset_initial_arm.set_start_point(enter_theta_);
                        result = reset_initial_arm.trajectory();
                        if (reset_initial_arm.get_complete()) {
                            reset_initial_arm.reset();
                            lift_mine.reset();
                        }
                    }
                }
                return false;
            },
            [this](const Auto_Sliver_Event& event, Auto_Sliver_Context& context) {},
            Auto_Sliver_State::Set_initial);
            fsm.start(Auto_Sliver_State::Set_initial);
    }
    Auto_Sliver_State getState() { return fsm.getCurrentState(); }
    void get_current_theta(std::array<double, 6> theta) { enter_theta_ = theta; }
    std::array<double, 6> get_result() { return result; }
    bool get_first_trajectory_result() { return reset_initial_arm.get_complete(); }
    void reset_first_trajectory() { reset_initial_arm.reset(); }
    bool get_second_trajectory_result() { return lift_mine.get_complete(); }

    void processEvent(const Auto_Sliver_Event& event) { fsm.processEvent(event); }

    void reset() {
        fsm.start(Auto_Sliver_State::Set_initial);
        fsm_direction = initial_enter;

        reset_initial_arm.reset();
        lift_mine.reset();
    }
    int fsm_direction = initial_enter;

private:
    Auto_Sliver_Context context_;
    FiniteStateMachine<Auto_Sliver_State, Auto_Sliver_Event, Auto_Sliver_Context> fsm{context_};
    std::array<double, 6> enter_theta_, result;
    rmcs_core::hardware::device::Trajectory<rmcs_core::hardware::device::TrajectoryType::JOINT>
        reset_initial_arm;

    rmcs_core::hardware::device::Trajectory<rmcs_core::hardware::device::TrajectoryType::LINE>
        lift_mine;
};