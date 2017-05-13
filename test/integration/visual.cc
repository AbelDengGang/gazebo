/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <mutex>
#include <functional>

#include <ignition/math/Rand.hh>

#include "gazebo/physics/physics.hh"
#include "gazebo/sensors/sensors.hh"
#include "gazebo/common/Timer.hh"
#include "gazebo/rendering/Camera.hh"
#include "gazebo/sensors/CameraSensor.hh"

#include "gazebo/test/ServerFixture.hh"

using namespace gazebo;
class VisualProperty : public ServerFixture
{
};

std::mutex mutex;

unsigned char* img = nullptr;
unsigned char* img2 = nullptr;
int imageCount = 0;
int imageCount2 = 0;

/////////////////////////////////////////////////
void OnNewCameraFrame(int* _imageCounter, unsigned char* _imageDest,
                  const unsigned char *_image,
                  unsigned int _width, unsigned int _height,
                  unsigned int _depth,
                  const std::string &/*_format*/)
{
  std::lock_guard<std::mutex> lock(mutex);
  memcpy(_imageDest, _image, _width * _height * _depth);
  *_imageCounter += 1;
}

/////////////////////////////////////////////////
TEST_F(VisualProperty, CastShadows)
{
  Load("worlds/visual_shadows.world");

  // Make sure the render engine is available.
  if (rendering::RenderEngine::Instance()->GetRenderPathType() ==
      rendering::RenderEngine::NONE)
  {
    gzerr << "No rendering engine, unable to run camera test"
          << std::endl;
    return;
  }

  // spawn sensors of various sizes to test speed
  std::string modelName = "camera_model";
  std::string modelName2 = "camera_model2";
  std::string cameraName = "camera_sensor";
  std::string cameraName2 = "camera_sensor2";
  unsigned int width  = 320;
  unsigned int height = 240;
  double updateRate = 10;
  ignition::math::Pose3d testPose(
      ignition::math::Vector3d(0, 0, 0.5),
      ignition::math::Quaterniond(0, 1.57, 0));
  ignition::math::Pose3d testPose2(
      ignition::math::Vector3d(0, 10, 0.5),
      ignition::math::Quaterniond(0, 1.57, 0));

  SpawnCamera(modelName, cameraName, testPose.Pos(),
      testPose.Rot().Euler(), width, height, updateRate);
  SpawnCamera(modelName2, cameraName2, testPose2.Pos(),
      testPose2.Rot().Euler(), width, height, updateRate);

  sensors::SensorPtr sensor = sensors::get_sensor(cameraName);
  sensors::CameraSensorPtr camSensor =
    std::dynamic_pointer_cast<sensors::CameraSensor>(sensor);
  sensors::SensorPtr sensor2 = sensors::get_sensor(cameraName2);
  sensors::CameraSensorPtr camSensor2 =
    std::dynamic_pointer_cast<sensors::CameraSensor>(sensor2);

  imageCount = 0;
  imageCount2 = 0;
  img = new unsigned char[width * height * 3];
  img2 = new unsigned char[width * height * 3];

  event::ConnectionPtr c =
      camSensor->Camera()->ConnectNewImageFrame(
      std::bind(&::OnNewCameraFrame, &imageCount, img,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4, std::placeholders::_5));
  event::ConnectionPtr c2 =
      camSensor2->Camera()->ConnectNewImageFrame(
      std::bind(&::OnNewCameraFrame, &imageCount2, img2,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4, std::placeholders::_5));

  common::Timer timer;
  timer.Start();

  // wait for images
  int totalImages = 50;
  while (imageCount < totalImages && imageCount2 < totalImages &&
      timer.GetElapsed().Double() < 5)
    common::Time::MSleep(10);

  unsigned int colorSum = 0;
  unsigned int colorSum2 = 0;
  for (unsigned int i = 0; i < height*width*3; i+=3)
  {
    unsigned int r = img[i];
    unsigned int g = img[i + 1];
    unsigned int b = img[i + 2];
    colorSum += r + g + b;
    unsigned int r2 = img2[i];
    unsigned int g2 = img2[i + 1];
    unsigned int b2 = img2[i + 2];
    colorSum2 += r2 + g2 + b2;
  }
  std::cerr << "color sum " << colorSum << " vs " << colorSum2
    << std::endl;

  // camera1 image should be darker than camera2 image
  // because the mesh below camera1 is casting shadows
  EXPECT_LT(colorSum, colorSum2);
  double colorRatio = static_cast<double>(colorSum2-colorSum) /
      static_cast<double>(colorSum2);
  EXPECT_GT(colorRatio, 0.05);

  camSensor->Camera()->DisconnectNewImageFrame(c);
  camSensor2->Camera()->DisconnectNewImageFrame(c2);
  delete [] img;
  delete [] img2;
}

int main(int argc, char **argv)
{
  // Set a specific seed to avoid occasional test failures due to
  // statistically unlikely, but possible results.
  ignition::math::Rand::Seed(42);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}