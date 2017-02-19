#include "c2017/citrus_robot/main.h"
#include "WPILib.h"
#include "c2017/queue_manager/queue_manager.h"
#include "muan/wpilib/queue_types.h"

namespace c2017 {
namespace citrus_robot {

CitrusRobot::CitrusRobot() : throttle_{1}, wheel_{0}, gamepad_{2} {
  fender_align_shoot_ = throttle_.MakeButton(1);                                          // Joystick Trigger
  score_hp_gear_ = throttle_.MakeButton(2);                                               // Joystick Button

  ball_intake_toggle_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::RIGHT_BUMPER));  // Right Bumper
  ball_intake_run_ = gamepad_.MakeAxis(3);                                                // Right Trigger
  gear_intake_down_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::A_BUTTON));        // Button A
  ground_gear_score_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::B_BUTTON));       // Button B
  ball_reverse_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::X_BUTTON));            // Button X
  just_shoot_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::Y_BUTTON));              // Button Y
  stop_shooting_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::LEFT_BUMPER));        // Left bumper
  hp_load_gears_ = gamepad_.MakePov(0, muan::teleop::Pov::kNorth);                        // D-Pad up
  hp_load_balls_ = gamepad_.MakePov(0, muan::teleop::Pov::kSouth);                        // D-Pad down
  hp_load_both_ = gamepad_.MakePov(0, muan::teleop::Pov::kEast);                          // D-Pad right
  agitate_ = gamepad_.MakeAxis(2);                                                        // Left Trigger
  climb_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::BACK));                       // Back Button
  just_spinup_ = gamepad_.MakeButton(uint32_t(muan::teleop::XBox::START));                // Start Button
  quickturn_ = wheel_.MakeButton(5);
}

void CitrusRobot::Update() {
  if (DriverStation::GetInstance().IsAutonomous() && DriverStation::GetInstance().IsEnabled()) {
    lemonscript_.Start();  // Weird to call Start in a loop, but it's a setter so it's fine
  } else if (DriverStation::GetInstance().IsOperatorControl()) {
    lemonscript_.Stop();  // Weirder to do this, but it works :/

    // Update joysticks
    throttle_.Update();
    wheel_.Update();
    gamepad_.Update();
    SendDrivetrainMessage();
    SendSuperstructureMessage();
  }

  SendDSMessage();
}

void CitrusRobot::SendDSMessage() {
  muan::wpilib::DriverStationProto status;

  if (DriverStation::GetInstance().IsDisabled()) {
    status->set_mode(RobotMode::DISABLED);
  } else if (DriverStation::GetInstance().IsAutonomous()) {
    status->set_mode(RobotMode::AUTONOMOUS);
  } else if (DriverStation::GetInstance().IsOperatorControl()) {
    status->set_mode(RobotMode::TELEOP);
  } else {
    status->set_mode(RobotMode::ESTOP);
  }

  status->set_battery_voltage(DriverStation::GetInstance().GetBatteryVoltage());
  status->set_brownout(DriverStation::GetInstance().IsBrownedOut());
  status->set_has_ds_connection(DriverStation::GetInstance().IsDSAttached());

  c2017::QueueManager::GetInstance().driver_station_queue()->WriteMessage(status);
}

void CitrusRobot::SendSuperstructureMessage() {
  // Intake Buttons
  if (ball_intake_toggle_->was_clicked()) {
    // Kelly - Gamepad Button
    ball_intake_down_ = !ball_intake_down_;
    intake_group_goal->set_ground_intake_position(ball_intake_down_ ? intake_group::INTAKE_BALLS
                                                                    : intake_group::INTAKE_NONE);
    intake_group_goal->set_gear_intake(intake_group::GEAR_IDLE);
  } else if (ball_intake_run_->is_pressed()) {
    // Kelly - Gamepad Trigger
    intake_group_goal->set_roller(intake_group::ROLLERS_INTAKE);
    intake_group_goal->set_gear_intake(intake_group::GEAR_IDLE);
  } else if (gear_intake_down_->is_pressed()) {
    // Kelly - Gamepad Button
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_GEAR);
    intake_group_goal->set_gear_intake(intake_group::GEAR_INTAKE);
  } else if (ball_reverse_->is_pressed()) {
    // Kelly - Gamepad Button
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_BALLS);
    intake_group_goal->set_roller(intake_group::ROLLERS_OUTTAKE);
    intake_group_goal->set_gear_intake(intake_group::GEAR_IDLE);
  } else if (agitate_->is_pressed()) {
    // Kelly - Gamepad Trigger
    intake_group_goal->set_roller(intake_group::ROLLERS_AGITATE);
  } else {
    intake_group_goal->set_roller(intake_group::ROLLERS_IDLE);
    intake_group_goal->set_gear_intake(intake_group::GEAR_IDLE);
  }

  // Hp load buttons
  if (hp_load_gears_->is_pressed()) {
    // Kelly - Gamepad D-Pad
    intake_group_goal->set_hp_load_type(intake_group::HP_LOAD_GEAR);
  } else if (hp_load_balls_->is_pressed()) {
    // Kelly - Gamepad D-Pad
    intake_group_goal->set_hp_load_type(intake_group::HP_LOAD_BALLS);
  } else if (hp_load_both_->is_pressed()) {
    // Kelly - Gamepad D-Pad
    intake_group_goal->set_hp_load_type(intake_group::HP_LOAD_BOTH);
  } else {
    intake_group_goal->set_hp_load_type(intake_group::HP_LOAD_NONE);
  }

  // Gear score buttons
  if (ground_gear_score_->is_pressed()) {
    // Kelly - Gamepad Button
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_NONE);
    intake_group_goal->set_gear_intake(intake_group::GEAR_OUTTAKE);
  } else if (score_hp_gear_->is_pressed()) {
    // Avery - Throttle Button
    intake_group_goal->set_score_hp_gear(true);
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_BALLS);
    intake_group_goal->set_gear_intake(intake_group::GEAR_IDLE);
  } else if (score_hp_gear_->was_released()) {
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_NONE);
  } else {
    intake_group_goal->set_score_hp_gear(false);
  }

  if (climb_->was_released()) {
    // Kelly - Gamepad Button
    currently_climbing_ = !currently_climbing_;
    shooter_group_goal->set_should_climb(currently_climbing_);
  }

  // Shooting buttons
  if (fender_align_shoot_->was_clicked()) {
    // Avery - Throttle Button
    // intake_group_goal->set_ground_intake_position(intake_group::INTAKE_BALLS);
    // shooter_group_goal->set_position(shooter_group::Position::FENDER);
    // shooter_group_goal->set_wheel(shooter_group::Wheel::BOTH);
    shooter_group_goal->set_position(shooter_group::Position::FENDER);
    shooter_group_goal->set_wheel(shooter_group::Wheel::SPINUP);
    using_vision_ = true;
  } else if (just_spinup_->is_pressed()) {
    // Kelly - Gamepad Button
    shooter_group_goal->set_position(shooter_group::Position::FENDER);
    shooter_group_goal->set_wheel(shooter_group::Wheel::SPINUP);
    using_vision_ = false;
  } else if (just_shoot_->is_pressed()) {
    // Kelly - Gamepad Button
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_BALLS);
    shooter_group_goal->set_position(shooter_group::Position::FENDER);
    shooter_group_goal->set_wheel(shooter_group::Wheel::SHOOT);
    using_vision_ = false;
  } else if (stop_shooting_->was_clicked()) {
    // Kelly - Gamepad Button
    shooter_group_goal->set_wheel(shooter_group::Wheel::IDLE);
    intake_group_goal->set_roller(intake_group::ROLLERS_IDLE);
    intake_group_goal->set_ground_intake_position(intake_group::INTAKE_NONE);
    using_vision_ = false;
  }

  c2017::QueueManager::GetInstance().climber_goal_queue().WriteMessage(climber_goal);
  c2017::QueueManager::GetInstance().intake_group_goal_queue().WriteMessage(intake_group_goal);
  c2017::QueueManager::GetInstance().shooter_group_goal_queue().WriteMessage(shooter_group_goal);
}

void CitrusRobot::SendDrivetrainMessage() {
  c2017::vision::VisionGoalProto vision_goal;
  vision_goal->set_should_align(using_vision_);

  frc971::control_loops::drivetrain::GoalProto drivetrain_goal;

  double throttle = -throttle_.wpilib_joystick()->GetRawAxis(1);
  double wheel = -wheel_.wpilib_joystick()->GetRawAxis(0);
  bool quickturn = quickturn_->is_pressed();

  drivetrain_goal->mutable_teleop_command()->set_steering(wheel);
  drivetrain_goal->mutable_teleop_command()->set_throttle(throttle);
  drivetrain_goal->mutable_teleop_command()->set_quick_turn(quickturn);

  auto vision_status = c2017::QueueManager::GetInstance().vision_status_queue().ReadLastMessage();
  if (!using_vision_ || !vision_status || vision_status.value()->aligned()) {
    using_vision_ = false;
    c2017::QueueManager::GetInstance().drivetrain_goal_queue()->WriteMessage(drivetrain_goal);
  }
  c2017::QueueManager::GetInstance().vision_goal_queue().WriteMessage(vision_goal);
}

}  // namespace citrus_robot
}  // namespace c2017
