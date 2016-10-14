#include "scoop.h"
#include <iostream>

using namespace muan::control;
using namespace frc1678::scoop;

Scoop::Scoop() {
  auto plant = StateSpacePlant<1, 2, 1> (controller::A(), controller::B(), controller::C());
  controller_ = StateSpaceController<1, 2, 1> (controller::K());
  controller_.u_min() = Eigen::Matrix<double, 1, 1>::Ones() * -12;
  controller_.u_max() = Eigen::Matrix<double, 1, 1>::Ones() * 12;
  observer_ = StateSpaceObserver<1, 2, 1> (plant, controller::L());
}

Voltage Scoop::Update(Angle goal, Angle sensor_value) {
  Eigen::Matrix<double, 2, 1> r;
  auto y = (Eigen::Matrix<double, 1, 1>() << sensor_value).finished();
  r = (Eigen::Matrix<double, 2, 1>() << goal, 0.0).finished();
  auto u = controller_.Update(observer_.x(), r);
  observer_.Update(u, y);
  return u(0, 0);
}

Angle Scoop::get_angle() const {
  return observer_.x()(0, 0);
}

AngularVelocity Scoop::get_angular_velocity() const {
  return observer_.x()(1, 0);
}

void Scoop::set_angle(Angle theta) {
  observer_.x()(0, 0) = theta;
}
