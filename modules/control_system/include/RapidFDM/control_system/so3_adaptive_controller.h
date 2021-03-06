//
// Created by xuhao on 2017/3/23.
//

#ifndef RAPIDFDM_TCC_DYNAMICS_CONTROLLER_H
#define RAPIDFDM_TCC_DYNAMICS_CONTROLLER_H

#include <RapidFDM/control_system/base_controller.h>
#include "mat.h"
#include <random>
extern "C"
{
#include "L1AircraftControl.h"
}

namespace RapidFDM
{
    namespace ControlSystem{
        class so3_adaptive_controller: public BaseController
        {
        public:

            AdaptiveSysT sys = {0};
            AttitudeCtrlT ctrlAttitude;
            MATFile *pmat;

            std::vector<AdaptiveCtrlT> ctrl_log;
            std::vector<AdaptiveSysT> sys_log;
            std::vector<AttitudeCtrlT> att_con_log;
            std::string controller_type = "";

            so3_adaptive_controller(Aerodynamics::AircraftNode * _aircraftNode);
            virtual void control_multirotor(float deltatime);
            virtual void control_fixedwing(float deltatime);
            virtual void control_combinevtol(float deltatime);
            virtual void control_step(float deltatime) override ;

            std::default_random_engine generator;
            std::normal_distribution<double> ang_vel_dist;

            void save_data_file();

        };
    }
}

#endif //RAPIDFDM_TCC_DYNAMICS_CONTROLLER_H
