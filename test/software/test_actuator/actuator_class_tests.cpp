#include <unity.h>
#include <Actuator.h>

void setUp(void)
{

}

void tearDown(void)
{

}

void test_run_encoder_homing()
{
    Actuator actuator;
    TEST_ASSERT_TRUE(actuator.encoder_homing());

}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_run_encoder_homing);

    UNITY_END();
}