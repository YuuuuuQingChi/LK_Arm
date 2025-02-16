#pragma once

#include <algorithm>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include "cmath"
#include <rclcpp/logging.hpp>
#include <rclcpp/node.hpp>

namespace rmcs_core::hardware::device {

enum class TrajectoryType { LINE, BEZIER, JOINT };

template <TrajectoryType Type>
class Trajectory : public rclcpp::Node {
public:
    Trajectory()
        : Node("trajectory_node") {
        current_step = 1.0f;
        is_complete  = false;
    }

    Trajectory& set_start_point(std::array<double, 3> xyz, std::array<double, 3> rpy) {
        xyz_start = xyz;
        rpy_start = rpy;
        return *this;
    }
    template <
        TrajectoryType T = Type, typename std::enable_if<T == TrajectoryType::JOINT, int>::type = 0>
    Trajectory& set_start_point(std::array<double, 6> joint_angles) {
        joint_start = joint_angles;
        return *this;
    }

    Trajectory& set_end_point(std::array<double, 3> xyz, std::array<double, 3> rpy) {
        xyz_end = xyz;
        rpy_end = rpy;
        return *this;
    }
    template <
        TrajectoryType T = Type, typename std::enable_if<T == TrajectoryType::JOINT, int>::type = 0>
    Trajectory& set_end_point(std::array<double, 6> joint_angles) {
        joint_end = joint_angles;
        return *this;
    }

    Trajectory& set_control_points(std::array<double, 3> xyz1, std::array<double, 3> xyz2) {
        if constexpr (Type == TrajectoryType::BEZIER) {
            xyz_control_1 = xyz1;
            xyz_control_2 = xyz2;
        }
        return *this;
    }

    Trajectory& set_total_step(double total_step_) {
        total_step = total_step_;
        return *this;
    }

    void reset() {
        current_step = 1.0f;
        is_complete  = false;
    }

    bool get_complete() const { return is_complete; }

    std::array<double, 6> trajectory() {
        double alpha = (current_step - 1) / (total_step - 1);
        if (current_step == total_step + 1.0) {
            is_complete = true;
            // RCLCPP_INFO(
            //     this->get_logger(), "%f %f %f %f %f %f", result[0], result[1], result[2], result[3],
            //     result[4], result[5]);
        }

        if (current_step <= total_step) {

            if constexpr(Type == TrajectoryType::LINE || Type == TrajectoryType::BEZIER){
            Eigen::Vector3d xyz_result;
            if constexpr (Type == TrajectoryType::LINE) {
                Eigen::Vector3d xyz_start_eigen(xyz_start[0], xyz_start[1], xyz_start[2]);
                Eigen::Vector3d xyz_end_eigen(xyz_end[0], xyz_end[1], xyz_end[2]);
                xyz_result = (1 - alpha) * xyz_start_eigen + alpha * xyz_end_eigen;
            } else if constexpr (Type == TrajectoryType::BEZIER) {
                Eigen::Vector3d P0(xyz_start[0], xyz_start[1], xyz_start[2]);
                Eigen::Vector3d P3(xyz_end[0], xyz_end[1], xyz_end[2]);
                Eigen::Vector3d P1(xyz_control_1[0], xyz_control_1[1], xyz_control_1[2]);
                Eigen::Vector3d P2(xyz_control_2[0], xyz_control_2[1], xyz_control_2[2]);
                Eigen::Vector3d P01  = (1 - alpha) * P0 + alpha * P1;
                Eigen::Vector3d P12  = (1 - alpha) * P1 + alpha * P2;
                Eigen::Vector3d P23  = (1 - alpha) * P2 + alpha * P3;
                Eigen::Vector3d P012 = (1 - alpha) * P01 + alpha * P12;
                Eigen::Vector3d P123 = (1 - alpha) * P12 + alpha * P23;
                xyz_result           = (1 - alpha) * P012 + alpha * P123;
            }

            Eigen::Vector3d rpy_start_eigen(rpy_start[0], rpy_start[1], rpy_start[2]);
            Eigen::Vector3d rpy_end_eigen(rpy_end[0], rpy_end[1], rpy_end[2]);
            Eigen::Vector3d rpy_result = (1 - alpha) * rpy_start_eigen + alpha * rpy_end_eigen;

            std::copy(xyz_result.data(), xyz_result.data() + 3, result.begin());
            std::copy(rpy_result.data(), rpy_result.data() + 3, result.begin() + 3);
            }
            else if constexpr(Type == TrajectoryType::JOINT) {
                // double a0 = joint_start[0]
                Eigen::VectorXd result_(6),a0(6),a1(6),a2(6),a3(6),start(6),end(6);
                start << joint_start[0],joint_start[1],joint_start[2],joint_start[3],joint_start[4],joint_start[5];
                end << joint_end[0],joint_end[1],joint_end[2],joint_end[3],joint_end[4],joint_end[5];

                a0 = start;
                // a1 = Eigen::VectorXd::Zero(6); 
                a2 = 3.0f *(end - start) / pow(total_step,2);
                a3 = -2.0f * (end - start) / pow(total_step,3);
                result_ = a0 + a2 * pow(current_step,2) + a3 * pow(current_step, 3);
                std::copy(result_.data(),result_.data()+6,result.begin());
            }


            current_step++;
        }

        return result;
    }

private:
    double total_step;
    double current_step;
    bool is_complete;
    std::array<double, 3> xyz_start, xyz_end, xyz_control_1, xyz_control_2;
    std::array<double, 3> rpy_start, rpy_end;
    std::array<double, 6> result;

    std::array<double, 6> joint_start;
    std::array<double, 6> joint_end;
};

} // namespace rmcs_core::hardware::device