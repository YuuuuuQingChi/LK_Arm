#include "hardware/device/Kinematic.hpp"
#include "hardware/device/drag_teach.hpp"
#include "hardware/device/trajectory.hpp"
#include "hardware/fsm/FSM.hpp"
#include "hardware/fsm/FSM_gold_l.hpp"
#include "hardware/fsm/FSM_gold_m.hpp"
#include "hardware/fsm/FSM_gold_r.hpp"
#include "hardware/fsm/FSM_sliver.hpp"
#include "hardware/fsm/FSM_walk.hpp"
#include "rmcs_msgs/arm_mode.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <fstream>
#include <numbers>
#include <rclcpp/logging.hpp>
#include <rclcpp/node.hpp>
#include <rmcs_executor/component.hpp>
#include <rmcs_msgs/keyboard.hpp>
#include <rmcs_msgs/mouse.hpp>
#include <rmcs_msgs/switch.hpp>

namespace rmcs_core::controller::arm {
class ArmController
    : public rmcs_executor::Component
    , public rclcpp::Node {
public:
    ArmController()
        : Node(
              get_component_name(),
              rclcpp::NodeOptions{}.automatically_declare_parameters_from_overrides(true)) {

        register_input("/arm/Joint6/theta", theta[5]);
        register_input("/arm/Joint5/theta", theta[4]);
        register_input("/arm/Joint4/theta", theta[3]);
        register_input("/arm/Joint3/theta", theta[2]);
        register_input("/arm/Joint2/theta", theta[1]);
        register_input("/arm/Joint1/theta", theta[0]);

        register_input("/arm/Joint1/vision", vision_theta1);
        register_input("/arm/Joint2/vision", vision_theta2);
        register_input("/arm/Joint3/vision", vision_theta3);
        register_input("/arm/Joint4/vision", vision_theta4);
        register_input("/arm/Joint5/vision", vision_theta5);
        register_input("/arm/Joint6/vision", vision_theta6);
        register_input("/arm/customer", vision_connectation_);

        register_output("/arm/Joint6/target_theta", target_theta[5], nan);
        register_output("/arm/Joint5/target_theta", target_theta[4], nan);
        register_output("/arm/Joint4/target_theta", target_theta[3], nan);
        register_output("/arm/Joint3/target_theta", target_theta[2], nan);
        register_output("/arm/Joint2/target_theta", target_theta[1], nan);
        register_output("/arm/Joint1/target_theta", target_theta[0], nan);

        register_output("/arm/enable_flag", is_arm_enable, true);

       
    }
    void update() override {
        
      
        using namespace rmcs_msgs;
        if (!(*vision_connectation_)) {
            *is_arm_enable   = false;
            *target_theta[5] = *theta[5];
            *target_theta[4] = *theta[4];
            *target_theta[3] = *theta[3];
            *target_theta[2] = *theta[2];
            *target_theta[1] = *theta[1];
            *target_theta[0] = *theta[0];

        } else {
            *is_arm_enable = true;
            *target_theta[0] = *vision_theta1;
            *target_theta[1] = *vision_theta2;
            *target_theta[2] = *vision_theta3;
            *target_theta[3] = *vision_theta4;
            *target_theta[4] = *vision_theta5;
            *target_theta[5] = *vision_theta6;
        }      
    }

private:
   
    bool is_vision_exchange = false;

    static constexpr double nan = std::numeric_limits<double>::quiet_NaN();

  
    OutputInterface<bool> is_arm_enable;
    InputInterface<double> theta[6]; // motor_current_angle
    OutputInterface<double> target_theta[6];

    InputInterface<double> vision_theta1;
    InputInterface<double> vision_theta2;
    InputInterface<double> vision_theta3;
    InputInterface<double> vision_theta4;
    InputInterface<double> vision_theta5;
    InputInterface<double> vision_theta6;
    InputInterface<bool> vision_connectation_;
};
} // namespace rmcs_core::controller::arm
#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(rmcs_core::controller::arm::ArmController, rmcs_executor::Component)