//
// Created by xuhao on 2017/3/23.
//
#include <RapidFDM/control_system/so3_adaptive_controller.h>
#include <ctime>
#include <iomanip>
#include <L1ControlAttitude_types.h>

namespace RapidFDM {
    namespace ControlSystem {

        inline void copy_v3d_into_array(const Eigen::Vector3d v, double *arr) {
            arr[0] = v.x();
            arr[1] = v.y();
            arr[2] = v.z();
        }

        so3_adaptive_controller::so3_adaptive_controller(Aerodynamics::AircraftNode *_aircraftNode) :
                BaseController(_aircraftNode) {
            roll_sp = 0;
            pitch_sp = 0;
            //7 0.655
            //6 0.6
            //5 0.55
            //4 0.49
            //3 0.42
            init_attitude_controller(&ctrlAttitude);
            double lead_fc = 2.5;
            double lead_alpha = 5.5;
            double lag_fc = 5.5;
            double lag_alpha = 10.5;
            L1ControllerUpdateParams(&(ctrlAttitude.RollCtrl),7.0, 1.1, 32,1000,lag_fc,lag_alpha,lead_fc,lead_alpha) ;
            L1ControllerUpdateParams(&(ctrlAttitude.PitchCtrl),7.0, 1.1, 32,1000,lag_fc,lag_alpha,lead_fc,lead_alpha);
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "log/so3_log_%Y_%m_%d_%H_%M_%S.mat");
            auto file = oss.str();
            printf("Creating file %s...\n\n", file.c_str());
            pmat = matOpen(file.c_str(), "w");
        }

        void so3_adaptive_controller::control_step(float deltatime) {
            static int count = 0;
            count++;
            deltatime = deltatime / 1000;
            copy_v3d_into_array(angular_rate, sys.angular_rate);

            sys.quat[0] = quat.w();
            sys.quat[1] = quat.x();
            sys.quat[2] = quat.y();
            sys.quat[3] = quat.z();
            double u_roll = 0, u_pitch = 0;

            QuatControlSetpoint quatControlSetpoint;
            EulerControlSetPoint eulerControlSetPoint;
            quatControlSetpoint.yaw_sp_is_rate = 1;
            quatControlSetpoint.yaw_rate = 0;
            Eigen::Vector3d eul = quat2eulers(quat);
            Eigen::AngleAxisd rollAngle(roll_sp, Eigen::Vector3d::UnitX());
            Eigen::AngleAxisd pitchAngle(pitch_sp, Eigen::Vector3d::UnitY());
            Eigen::AngleAxisd yawAngle(eul.z(), Eigen::Vector3d::UnitZ());

            Eigen::Quaterniond quat = yawAngle * pitchAngle * rollAngle;
            quatControlSetpoint.quat[0] = quat.w();
            quatControlSetpoint.quat[1] = quat.x();
            quatControlSetpoint.quat[2] = quat.y();
            quatControlSetpoint.quat[3] = quat.z();

            eulerControlSetPoint.roll = roll_sp;
            eulerControlSetPoint.pitch = pitch_sp;
            eulerControlSetPoint.yaw = yaw_sp;
            eulerControlSetPoint.yaw_sp_is_rate = 1;

            L1ControlAttitude(&ctrlAttitude, deltatime, &quatControlSetpoint, &sys, &u_roll, &u_pitch);
//            L1ControlAttitudeEuler(&ctrlAttitude,deltatime,&eulerControlSetPoint,&sys,&u_roll,&u_pitch);

            Eigen::AngleAxisd rot_u(0 * M_PI / 180, Eigen::Vector3d::UnitZ());
            Eigen::Vector3d u = Eigen::Vector3d(0, 0, 0);
            Eigen::Vector3d u_last = Eigen::Vector3d(0, 0, 0);
            u.x() = u_roll;
            u.y() = u_pitch;
            u.z() = yaw_sp;

//            u = u + (u - u_last) * 2;
//            u_last = u;
//            u = rot_u * u * 0.3;
//            pwm[0] = (float) u.x();
//            pwm[1] = (float) u.y();
//            pwm[2] = (float) throttle_sp;
//            pwm[3] = (float) yaw_sp;
            pwm[0] = (float) float_constrain((-u.x() + u.y() + u.z()) + throttle_sp, 0, 1);
            pwm[1] = (float) float_constrain((u.x() + u.y() - u.z()) + throttle_sp, 0, 1);
            pwm[2] = (float) float_constrain((u.x() - u.y() + u.z()) + throttle_sp, 0, 1);
            pwm[3] = (float) float_constrain((-u.x() - u.y() - u.z()) + throttle_sp, 0, 1);


            aircraftNode->set_control_from_channels(pwm, 8);

            ctrl_log.push_back(ctrlAttitude.RollCtrl);
            sys_log.push_back(sys);
            att_con_log.push_back(ctrlAttitude);

            if (count % 200 == 0) {
                save_data_file();
            }
        }

        void set_value_mx_array(mxArray *arr, int i, int j, double data) {
            size_t mi = mxGetM(arr);
            size_t ni = mxGetN(arr);//4
            *(mxGetPr(arr) + i + j * mi) = data;
        }

        void so3_adaptive_controller::save_data_file() {
            static int log_number = 0;
            mxArray *pa1, *pa2;
            //t 1 x 6 err 2 u 1 eta 1

            int cols = 22;
            pa1 = mxCreateDoubleMatrix(ctrl_log.size(), cols, mxREAL);
            for (int i = 0; i < ctrl_log.size(); i++) {
                const AdaptiveCtrlT &ctrlT = ctrl_log[i];
                set_value_mx_array(pa1, i, 0, ctrlT.t);
                for (int j = 0; j < 7; j++) {
                    set_value_mx_array(pa1, i, j + 1, ctrlT.x[j]);
                }
                set_value_mx_array(pa1, i, 8, ctrlT.err[0]);
                set_value_mx_array(pa1, i, 9, ctrlT.err[1]);
                set_value_mx_array(pa1, i, 10, ctrlT.u);
                set_value_mx_array(pa1, i, 11, ctrlT.eta);
                set_value_mx_array(pa1, i, 12, ctrlT.r);
                set_value_mx_array(pa1, i, 13, ctrlT.g[0]);
                for (int j = 0; j < 4; j++) {
                    set_value_mx_array(pa1, i, j + 14, att_con_log[i].quat[j]);
                }
                for (int j = 0; j < 4; j++) {
                    set_value_mx_array(pa1, i, j + 18, att_con_log[i].quat_sp[j]);
                }
            }
            char matname[100] = {0};
            sprintf(matname, "AdaptiveCtrlT_%d", log_number);
            matPutVariable(pmat, matname, pa1);
            mxDestroyArray(pa1);
            ctrl_log.clear();
            sys_log.clear();
            att_con_log.clear();
            log_number++;
        }
    }
}
