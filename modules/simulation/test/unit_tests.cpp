//
// Created by Hao Xu on 16/6/12.
//

#include <RapidFDM/simulation/utils.h>
#include <RapidFDM/simulation/simulator_world.h>
#include <RapidFDM/control_system/control_system.h>
#include <RapidFDM/simulation/simulator_aircraft.h>
#include <RapidFDM/aerodynamics/airdynamics_parser.h>
#include <thread>
#include <boost/asio.hpp>

using namespace RapidFDM::Simulation::Utils;
using namespace RapidFDM::Simulation;
using namespace RapidFDM::ControlSystem;
using namespace Eigen;


void convert_tests()
{
    Eigen::Vector3d fuck(1, 2, 3);
    PxVec3 vec3 = vector_e2p(fuck);
    vec3 = vec3 + PxVec3(3, 5, 6);
    fuck = vector_p2e(vec3);
    printf("Vec convert res : %5lf %5lf %5lf\n", fuck.x(), fuck.y(), fuck.z());

    Eigen::Quaterniond quaterniond_ori(
            Eigen::AngleAxisd(0.25 * M_PI, Vector3d::UnitX())
    );
    Eigen::Quaterniond quaterniond = quaterniond_ori;

    printf("Quat convert res 0 : wxyz %5lf %5lf %5lf %5lf\n",
           quaterniond.w(),
           quaterniond.x(),
           quaterniond.y(),
           quaterniond.z()
    );
    PxQuat pquat = quat_e2p(quaterniond);

    quaterniond = quaterniond * quat_p2e(pquat.getConjugate());

    printf("Quat convert res: wxyz %5lf %5lf %5lf %5lf\n",
           quaterniond.w(),
           quaterniond.x(),
           quaterniond.y(),
           quaterniond.z()
    );

    Affine3d trans;
    trans.fromPositionOrientationScale(
            fuck,
            quaterniond_ori,
            Vector3d(1, 1, 1)
    );
    printf("origin vec eigen %f %f %f px %f %f %f\n",
           fuck.x(), fuck.y(), fuck.z(),
           vec3.x, vec3.y, vec3.z
    );
    Vector3d eigen_transed = trans * fuck;
    printf("Eigen Trans :%5lf %5lf %5lf\n",
           eigen_transed.x(),
           eigen_transed.y(),
           eigen_transed.z()
    );

    PxTransform pxTransform = transform_e2p(trans);
    PxVec3 new_vec3 = pxTransform.transform(vec3);
    printf("Px Trans   :%5lf %5lf %5lf\n",
           new_vec3.x,
           new_vec3.y,
           new_vec3.z
    );

    trans = transform_p2e(pxTransform);
    eigen_transed = (trans * trans) * fuck;
    printf("Eigen Trans twice:%5lf %5lf %5lf\n",
           eigen_transed.x(),
           eigen_transed.y(),
           eigen_transed.z()
    );

    pxTransform = pxTransform.transform(pxTransform);
    new_vec3 = pxTransform.transform(vec3);

    printf("Px Trans twice:%5lf %5lf %5lf\n",
           new_vec3.x,
           new_vec3.y,
           new_vec3.z
    );


}


void test_construct_aircraft()
{
    SimulatorWorld world(0.01);
    RapidFDM::Aerodynamics::parser *parser1 = new RapidFDM::Aerodynamics::parser(
            "/Users/xuhao/Develop/FixedwingProj/RapidFDM/sample_data/aircrafts/sample_aircraft");
    BaseController *controller = new BaseController(parser1->get_aircraft_node());
    printf("\nTrying to construct simulator aircraft\n");
    SimulatorAircraft *simulatorAircraft = world.create_aircraft(parser1->get_aircraft_node(), controller);
    AircraftNode *node = simulatorAircraft->get_aircraft_node();
    node->set_internal_state("main_engine_0/n", 180);
    printf("Simulator aircraft construct successfully\n");
    for (int i = 0; i < 100000; i++) {
        world.Step(0.05);
    }

}

void handler(
        const boost::system::error_code &error,
        int signal_number)
{
    if (!error) {
        // A signal occurred.
            printf("Good bye world\n");
        abort();
    }
        printf("Good bye world2\n");
    abort();
}

int main()
{
    printf("Hello,world2\n");
    boost::asio::io_service io_service;
    boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
    signals.async_wait(handler);
    new std::thread([&] {
        test_construct_aircraft();
    });
    io_service.run();
    printf("Test finish\n");
}

