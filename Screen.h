#ifndef screen_h
#define screen_h

#include <LiquidCrystal_I2C.h>

const int lcdColumns = 16;
const int lcdRows = 2;
const int lcdAddress = 0x27;

const int buffer_size = 4;

struct Message {
  uint16_t duration_ms = 0;
  char buffer[17];
};

class Screen {
  private:
    LiquidCrystal_I2C lcd;
    struct timeval time;
    Message messages[buffer_size];
    uint8_t head = 0;
    uint8_t tail = 0;
    struct timeval last_updated;
    char time_buffer[17];

    void update_time() {
      if (time.tv_sec == 0)
        return;
      struct tm *gmt = gmtime((time_t*)(&time.tv_sec));
      strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", gmt);
      lcd.setCursor(0, 0);
      lcd.print(time_buffer);
      lcd.setCursor(8, 0);
      lcd.print('.');
      lcd.setCursor(9, 0);
      uint16_t ms = time.tv_usec / 1e3;
      lcd.print(ms);
    }

    void clear_bottom_row() {
      lcd.clear();
      update_time();
    }

    void display_current_message() {
      clear_bottom_row();
      lcd.setCursor(0, 1);
      lcd.print(messages[head].buffer);
    }

    void advance_message() {
      if (head == tail && messages[head].duration_ms == 0)
        return;

      struct timeval now, diff;
      gettimeofday(&now, NULL);
      timersub(&now, &last_updated, &diff);
      uint32_t ms = diff.tv_sec * 1e3 + diff.tv_usec / 1e3;
      if (ms >= messages[head].duration_ms) {
        if (head != tail) {
          head = (head + 1) % buffer_size;
          display_current_message();
        }
        else {
          clear_bottom_row();
          messages[head].duration_ms = 0;
        }
      }
    }

  public:
    Screen() : lcd(lcdAddress, lcdColumns, lcdRows) {
    }

    void init() {
      lcd.init();
      lcd.backlight();
      time.tv_sec = 0;
      last_updated.tv_sec = 0;
      last_updated.tv_usec = 0;
    }
    
    void setTime(struct timeval &time) {
      this->time = time;
    }

    int showMessage(char message[], uint16_t duration_ms = 0) {
      if (strlen(message) > lcdColumns)
        return -1;

      tail = (tail + 1) % buffer_size;
      messages[tail].duration_ms = duration_ms;
      strcpy(messages[tail].buffer, message);
      tick();

      return 0;
    }

    void tick() {
      update_time();

      advance_message();
    }
};

#endif
