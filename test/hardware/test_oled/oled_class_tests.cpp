#include <OLED.h>
#include <unity.h>


void test_display()
{
    OLED oled;
    TEST_ASSERT_TRUE( oled.init() );
    TEST_ASSERT_TRUE( oled.refresh() );
}

int main(int argc, char **argv)
{    
    UNITY_BEGIN();

    RUN_TEST(test_display);

    UNITY_END();
}