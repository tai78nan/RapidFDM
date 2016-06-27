//
// Created by Hao Xu on 16/6/27.
//

#ifndef RAPIDFDM_EASY_PROPELLER_H
#define RAPIDFDM_EASY_PROPELLER_H

#include <RapidFDM/aerodynamics/nodes/Node.h>
//TODO:
// rho
#define air_rho 1.29

namespace RapidFDM
{
    namespace Aerodynamics
    {
        //! This node stand for a electrical_propeller, no need battery and small accurate time
        //  this propeller perfermance spline is independce beyond D and H
        class EasyPropellerNode : public Node
        {
        protected:
            //Ct = A_ct * J + B_ct;
            //T = rho * n^2 * D^4 * Ct
            //Cq = 0.05 / 2pi
            //Q = rho * n^2 * D^5 * Cq
            double A_ct = -0.15;
            double B_ct = 0.09;
            double C_q = 0.007961783439;
            double D;
            //!Direction = 1 means 顺时针 产生负力矩
            int direction = 1;
            virtual float get_cq()
            {
                return C_q;
            }
        public:
            virtual float get_propeller_force()
            {
                double wind_speed = get_air_velocity().x();
                double n = inertial_states["n"];
                double J = wind_speed / n / D;
                double Ct = A_ct * J + B_ct;
                return Ct * air_rho  * n * n *pow(D,4) ;
            }

            virtual float get_propeller_torque()
            {
                double n = inertial_states["n"];
                return - get_cq() * air_rho  * n * n *pow(D,5) * direction;
            }

            virtual Eigen::Vector3d get_realtime_force() override
            {
                //Force to positive x axis
                return Eigen::Vector3d(get_propeller_force(),0,0);
            }
            virtual Eigen::Vector3d get_realtime_torque() override
            {
                //Torque at x axis
                return Eigen::Vector3d(get_propeller_torque(),0,0);
            }



            EasyPropellerNode(rapidjson::Document &document) :
                    Node(document)
            {
                if (document.HasMember("geometry") && document["geometry"].IsObject()) {
                    this->geometry = GeometryHelper::create_geometry_from_json(document["geometry"]);
                }
                else {
                    this->geometry = new BaseGeometry();
                }
                D = fast_value(document, "D", 0.254);
                direction = fast_value(document,"direction",1);
                this->type = "propeller_node";
                printf("Success parse propeller_node\n");
                printf("Name %s type: %s geometry %s\n", this->name.c_str(), this->type.c_str(),
                       geometry->get_type().c_str());

            }
        };
    }
}
#endif //RAPIDFDM_ELECTRICAL_PROPELLER_H
