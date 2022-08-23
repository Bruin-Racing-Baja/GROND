#include <unity.h>
#include <Actuator.h>
#include <Odrive.h>

float test_velocity = 2.0;
int test_axis = ACTUATOR_AXIS;

void setUp(void)
{
    
}

void tearDown(void)
{

}

void test_actuator_connection()
{
    Odrive odrive(Serial1);
    Actuator actuator(&odrive);
    TEST_ASSERT_TRUE( 0 == actuator.init() );
}

void test_actuator_index_search()
{
    // Setup
    Odrive odrive(Serial1);
    Actuator actuator(&odrive);
    TEST_ASSERT_TRUE( 0 == actuator.init() );

    TEST_ASSERT_TRUE( actuator.encoder_index_search() );
}

void test_actuator_update_speed()
{
    // Setup
    Odrive odrive(Serial1);
    Actuator actuator(&odrive);
    TEST_ASSERT_TRUE( 0 == actuator.init() );
    TEST_ASSERT_TRUE( actuator.encoder_index_search() );

    TEST_ASSERT_TRUE( actuator.update_speed(test_velocity) == test_velocity );
    TEST_ASSERT_TRUE( actuator.get_current_speed() == test_velocity );
    delay(100);

    TEST_ASSERT_TRUE( actuator.update_speed(0.0) == 0.0 );
    TEST_ASSERT_TRUE( actuator.get_current_speed() == 0.0 );
}

void verify_movement_occurs()
{
    // Setup
    Odrive odrive(Serial1);
    Actuator actuator(&odrive);
    TEST_ASSERT_TRUE( 0 == actuator.init() );
    TEST_ASSERT_TRUE( actuator.encoder_index_search() );
    // Note: Accessing odrive object directly usually isn't the best idea
    // However, this is just for a test, and not for actual "production" code so the rules are a bit more lax
    int starting_count = odrive.get_encoder_count(test_axis);

    TEST_ASSERT_TRUE( actuator.update_speed(test_velocity) == test_velocity );
    TEST_ASSERT_TRUE( actuator.get_current_speed() == test_velocity );
    delay(100);

    TEST_ASSERT_TRUE( actuator.update_speed(0.0) == 0.0 );
    TEST_ASSERT_TRUE( actuator.get_current_speed() == 0.0 );
    TEST_ASSERT_TRUE( odrive.get_encoder_count(test_axis) != starting_count );
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_actuator_connection);

    UNITY_END();
}