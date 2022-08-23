#include <unity.h>
#include <Actuator.h>
#include <Odrive.h>

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

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_actuator_connection);

    UNITY_END();
}