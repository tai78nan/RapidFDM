//
// Created by xuhao on 2016/5/2.
//

#ifndef RAPIDFDM_NODE_H
#define RAPIDFDM_NODE_H

#include <Eigen/Eigen>
#include <vector>
#include <airdynamics/include/joints/Joint.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <FlyingData.h>
#include <base_component.h>
#include <geometrys/base_geometry.h>
#include <stdio.h>

namespace RapidFDM
{

    namespace Aerodynamics
    {
        class Joint;

        class Node : public BaseComponent
        {

        protected:
            std::vector<Joint*> linked_joints; /*!< List of linked joints*/

            struct {
                double mass; /*!< Mass of this node, in kilogram*/
                Eigen::Vector3d mass_center;/*!< Mass center to the center*/
                Eigen::Vector3d Inertial;/*!< Inertial for this*/
            } params;

            ComponentData flying_states;

            BaseGeometry *geometry = nullptr;

            Joint * parent = nullptr; /*!< Parent joint of this, be null if standalone */

            bool inSimulate = false;

            std::string type = "node";
        public:

            Node(Joint *_parent = nullptr);

            Node(rapidjson::Document &document, Joint *_parent = nullptr);

            //! Calucate total force of this node
            /*!
              \return The calucated realtime force
            */
            virtual Eigen::Vector3d get_realtime_force() {
                abort();
            };

            //! A Calucate total torque of this node
            /*!
              \return The calucated realtime torque
            */
            virtual Eigen::Vector3d get_realtime_torque() {
                abort();
            }

            Node(rapidjson::Value &_json, Joint *_parent = nullptr);


            //Overrides
            virtual Eigen::Quaterniond get_ground_attitude() override;

            virtual Eigen::Affine3d get_body_transform() override;

            virtual Eigen::Affine3d get_ground_transform() override;

            virtual Eigen::Vector3d get_ground_velocity() override;

            virtual Eigen::Vector3d get_air_velocity() override;

            virtual Eigen::Vector3d get_angular_velocity() override;

            virtual void set_mass(double _mass, Eigen::Vector3d mass_center = Eigen::Vector3d(0, 0, 0))
            {
                params.mass = _mass;
                params.mass_center = mass_center;
            }

            virtual void setSimulate(bool EnableSimulator) {
                this->inSimulate = EnableSimulator;
            }

            virtual void setSetfromsimulator(ComponentData flyingstates) {
                this->flying_states = flyingstates;
            };

            virtual void brief() override {
                printf("name : %s \n", name.c_str());
                printf("type :%s \n", type.c_str());
                printf("mass : %5f \n", params.mass);
                printf("Inertial %5f %5f %5f \n", params.Inertial.x(), params.Inertial.y(), params.Inertial.z());
            }

        };
    }
}


#endif //RAPIDFDM_NODE_H
