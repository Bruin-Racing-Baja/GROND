#ifndef oled_h
#define oled_h

class OLED
{
    public:
        bool init();
        bool refresh();
        bool printInt(int n);
    private:
        void setFormat();
};

#endif