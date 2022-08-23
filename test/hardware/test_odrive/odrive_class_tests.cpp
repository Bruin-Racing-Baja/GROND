#include <unity.h>
#include <Actuator.h>
#include <Odrive.h>

void setUp(void)
{
    
}

void tearDown(void)
{

}

void test_connection()
{
    Odrive odrive(Serial1);
    TEST_ASSERT_TRUE( 0 == odrive.init_connection() );
}

void test_vbus_value()
{
    Odrive odrive(Serial1);
    float bus_voltage = odrive.get_bus_voltage();
    TEST_ASSERT_FLOAT_WITHIN(2.0, 24.0, bus_voltage);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_connection);
    RUN_TEST(test_vbus_value);

    UNITY_END();
}