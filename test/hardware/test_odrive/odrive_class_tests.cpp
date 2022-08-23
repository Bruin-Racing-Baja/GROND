#include <Constants.h>
#include <Odrive.h>
#include <unity.h>

// Test constants
float test_actuator_velocity = 2.0;  //rps
int test_axis = ACTUATOR_AXIS;


void setUp(void)
{

}

void tearDown(void)
{

}

void test_connection()
{
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( odrive.init_connection() );
}

void test_vbus_value()
{
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( odrive.init_connection() );
    float bus_voltage = odrive.get_bus_voltage();
    TEST_ASSERT_FLOAT_WITHIN(2.0, 24.0, bus_voltage);
}

void test_encoder_index_search()
{
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( odrive.init_connection() );
    TEST_ASSERT_TRUE( odrive.encoder_index_search(test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_IDLE_STATE );
}

void test_set_velocity_and_idle()
{
    // Setup
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( odrive.encoder_index_search(test_axis) );

    // Turn on then off
    TEST_ASSERT_TRUE( odrive.set_velocity(test_actuator_velocity, test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_VELOCITY_CONTROL_STATE );
    delay(100);
    TEST_ASSERT_TRUE( odrive.set_velocity(0.0, test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_VELOCITY_CONTROL_STATE );
    
    // Test idle
    TEST_ASSERT_TRUE( odrive.idle(test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_IDLE_STATE );
}

void test_velocity_movement_and_encoder_count()
{
    // Setup
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( odrive.encoder_index_search(test_axis) );
    int starting_count = odrive.get_encoder_count(test_axis);

    // Turn on then off
    TEST_ASSERT_TRUE( odrive.set_velocity(test_actuator_velocity, test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_VELOCITY_CONTROL_STATE );
    delay(100);
    TEST_ASSERT_TRUE( odrive.set_velocity(0.0, test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_VELOCITY_CONTROL_STATE );

    TEST_ASSERT_TRUE( starting_count != odrive.get_encoder_count(test_axis) );

    TEST_ASSERT_TRUE( odrive.idle(test_axis) );
    TEST_ASSERT_TRUE( odrive.get_state(test_axis) == ODRIVE_IDLE_STATE );

}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_connection);
    RUN_TEST(test_vbus_value);
    RUN_TEST(test_encoder_index_search);

    UNITY_END();
}