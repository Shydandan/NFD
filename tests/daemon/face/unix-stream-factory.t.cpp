/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "face/unix-stream-factory.hpp"

#include "face-system-fixture.hpp"
#include "factory-test-common.hpp"

namespace nfd {
namespace face {
namespace tests {

#define CHANNEL_PATH1 "unix-stream-test.1.sock"
#define CHANNEL_PATH2 "unix-stream-test.2.sock"

using UnixStreamFactoryFixture = FaceSystemFactoryFixture<UnixStreamFactory>;

BOOST_AUTO_TEST_SUITE(Face)
BOOST_FIXTURE_TEST_SUITE(TestUnixStreamFactory, UnixStreamFactoryFixture)

using nfd::Face;

BOOST_AUTO_TEST_SUITE(ProcessConfig)

BOOST_AUTO_TEST_CASE(Normal)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      unix
      {
        path /tmp/nfd.sock
      }
    }
  )CONFIG";

  parseConfig(CONFIG, true);
  parseConfig(CONFIG, false);

  auto& factory = this->getFactoryById<UnixStreamFactory>("unix");
  BOOST_CHECK_EQUAL(factory.getChannels().size(), 1);
}

BOOST_AUTO_TEST_CASE(Omitted)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
    }
  )CONFIG";

  parseConfig(CONFIG, true);
  parseConfig(CONFIG, false);

  auto& factory = this->getFactoryById<UnixStreamFactory>("unix");
  BOOST_CHECK_EQUAL(factory.getChannels().size(), 0);
}

BOOST_AUTO_TEST_CASE(UnknownOption)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      unix
      {
        hello
      }
    }
  )CONFIG";

  BOOST_CHECK_THROW(parseConfig(CONFIG, true), ConfigFile::Error);
  BOOST_CHECK_THROW(parseConfig(CONFIG, false), ConfigFile::Error);
}

BOOST_AUTO_TEST_SUITE_END() // ProcessConfig

BOOST_AUTO_TEST_CASE(GetChannels)
{
  BOOST_CHECK_EQUAL(factory.getChannels().empty(), true);

  std::set<std::string> expected;
  expected.insert(factory.createChannel(CHANNEL_PATH1)->getUri().toString());
  expected.insert(factory.createChannel(CHANNEL_PATH2)->getUri().toString());
  checkChannelListEqual(factory, expected);
}

BOOST_AUTO_TEST_CASE(CreateChannel)
{
  auto channel1 = factory.createChannel(CHANNEL_PATH1);
  auto channel1a = factory.createChannel(CHANNEL_PATH1);
  BOOST_CHECK_EQUAL(channel1, channel1a);
  std::string uri = channel1->getUri().toString();
  BOOST_CHECK_EQUAL(uri.find("unix:///"), 0); // third '/' is the path separator
  BOOST_CHECK_EQUAL(uri.rfind(CHANNEL_PATH1),
                    uri.size() - std::string(CHANNEL_PATH1).size());

  auto channel2 = factory.createChannel(CHANNEL_PATH2);
  BOOST_CHECK_NE(channel1, channel2);
}

BOOST_AUTO_TEST_CASE(UnsupportedCreateFace)
{
  createFace(factory,
             FaceUri("unix:///var/run/nfd.sock"),
             {},
             ndn::nfd::FACE_PERSISTENCY_PERMANENT,
             false,
             {CreateFaceExpectedResult::FAILURE, 406, "Unsupported protocol"});

  createFace(factory,
             FaceUri("unix:///var/run/nfd.sock"),
             {},
             ndn::nfd::FACE_PERSISTENCY_ON_DEMAND,
             false,
             {CreateFaceExpectedResult::FAILURE, 406, "Unsupported protocol"});

  createFace(factory,
             FaceUri("unix:///var/run/nfd.sock"),
             {},
             ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
             false,
             {CreateFaceExpectedResult::FAILURE, 406, "Unsupported protocol"});
}

BOOST_AUTO_TEST_SUITE_END() // TestUnixStreamFactory
BOOST_AUTO_TEST_SUITE_END() // Face

} // namespace tests
} // namespace face
} // namespace nfd
