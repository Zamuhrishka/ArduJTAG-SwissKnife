// #include "file_to_test.h"
#include "unity.h"

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void test_function_should_doBlahAndBlah(void)
{
  // test stuff
}

void test_function_should_doAlsoDoBlah(void)
{
  // more test stuff
}

int runUnityTests(void)
{
  UNITY_BEGIN();
  //   RUN_TEST(test_function_should_doBlahAndBlah);
  //   RUN_TEST(test_function_should_doAlsoDoBlah);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void)
{
  return runUnityTests();
}

/**
 * For Arduino framework
 */
void setup()
{
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  //   delay(2000);

  runUnityTests();
}
void loop() {}

/**
 * For ESP-IDF framework
 */
void app_main()
{
  runUnityTests();
}

// #include <gtest/gtest.h>
// // uncomment line below if you plan to use GMock
// // #include <gmock/gmock.h>

// // TEST(...)
// // TEST_F(...)

// #if defined(ARDUINO)
//   #include <Arduino.h>

// void setup()
// {
//   // should be the same value as for the `test_speed` option in "platformio.ini"
//   // default value is test_speed=115200
//   Serial.begin(115200);

//   ::testing::InitGoogleTest();
//   // if you plan to use GMock, replace the line above with
//   // ::testing::InitGoogleMock();
// }

// void loop()
// {
//   // Run tests
//   if (RUN_ALL_TESTS())
//     ;

//   // sleep for 1 sec
//   delay(1000);
// }

// #else
// int main(int argc, char **argv)
// {
//   ::testing::InitGoogleTest(&argc, argv);
//   // if you plan to use GMock, replace the line above with
//   // ::testing::InitGoogleMock(&argc, argv);

//   if (RUN_ALL_TESTS())
//     ;

//   // Always return zero-code and allow PlatformIO to parse results
//   return 0;
// }
// #endif
