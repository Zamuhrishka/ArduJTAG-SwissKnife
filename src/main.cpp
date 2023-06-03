//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>

#include <assert.h>
//_____ C O N F I G S  ________________________________________________________

//_____ D E F I N I T I O N S _________________________________________________
/* JTAG and digital pins functionality */
enum jtag_pins {
  TCK = 0,
  TMS = 1,
  TDI = 2,
  TDO = 3,
  TRST = 4,

  N_JTAG_PINS = 5,
};

enum jtag_errors {
  JTAG_NO_ERROR = 0,
  JTAG_ERROR_BAD_PIN = -1,
  JTAG_ERROR_BAD_SPEED = -2,
  JTAG_ERROR_BAD_SEQUENCE_LEN = -3,
};

enum jtag_constants {
  JTAG_MAX_SPEED_KHZ = 500,
  JTAG_MAX_SEQUENCE_LEN = 256,
  JTAG_MAX_SEQUENCE_LEN_BYTES = JTAG_MAX_SEQUENCE_LEN / 8, // 32
};
//_____ M A C R O S ___________________________________________________________

//_____ V A R I A B L E S _____________________________________________________
int jtag_pin_map[N_JTAG_PINS] = {2, 3, 4, 5, 6};
const int jtag_pin_dir[N_JTAG_PINS] = {OUTPUT, OUTPUT, OUTPUT, INPUT, OUTPUT};
unsigned long jtag_last_tck_micros;
unsigned long jtag_min_tck_micros;
//_____ P R I V A T E  F U N C T I O N S_______________________________________

//_____ P U B L I C  F U N C T I O N S_________________________________________
/**
 * \brief Set or clear selected bit in array
 *
 * \param[in] i_bit: bit number
 * \param[inout] data: bit array
 * \param[in] value: set or clear
 */
void set_bit_in_array(int i_bit, byte *data, int value)
{
  int i_byte;
  byte mask;

  i_byte = i_bit >> 3; // floor(i_bit/8)
  mask = 1 << (i_bit & 0x7);

  if (value == 0) {
    data[i_byte] &= ~mask;
  } else {
    data[i_byte] |= mask;
  }
}

/**
 * \brief Get the selected bit value in array
 *
 * \param[in] i_bit: bit number
 * \param[in] data: bit array
 *
 * \return bit state
 */
int get_bit_from_array(int i_bit, const byte *data)
{
  int i_byte;
  byte mask;

  i_byte = i_bit >> 3; // floor(i_bit/8)
  mask = 1 << (i_bit & 0x7);

  return ((data[i_byte] & mask) == 0) ? 0 : 1;
}


void jtag_setup()
{
  int i_pin;
  for (i_pin = 0; i_pin < N_JTAG_PINS; i_pin++)
  {
    digitalWrite(jtag_pin_map[i_pin], LOW);
    pinMode(jtag_pin_map[i_pin], jtag_pin_dir[i_pin]);
  }

  jtag_last_tck_micros = micros();
  jtag_min_tck_micros = 1;
}



int jtag_pin_write(int pin, int value)
{
  if (pin >= N_JTAG_PINS || pin < 0) {
    return JTAG_ERROR_BAD_PIN;
  }

  if (jtag_pin_dir[pin] != OUTPUT) {
    return JTAG_ERROR_BAD_PIN;
  }

  digitalWrite(jtag_pin_map[pin], value);

  return JTAG_NO_ERROR;
}

int jtag_pin_set(int pin) { return jtag_pin_write(pin, HIGH); }

int jtag_pin_clear(int pin) { return jtag_pin_write(pin, LOW); }

int jtag_pin_get(int pin)
{
  if (pin >= N_JTAG_PINS || pin < 0) {
    return JTAG_ERROR_BAD_PIN;
  }

  if (jtag_pin_dir[pin] != INPUT) {
    return JTAG_ERROR_BAD_PIN;
  }

  if (digitalRead(jtag_pin_map[pin]) == HIGH)
    return 1;
  else
    return 0;
}

int jtag_pulse_high(int pin, unsigned int us)
{
  int jtag_error;

  jtag_error = jtag_pin_set(pin);

  if (jtag_error != JTAG_NO_ERROR) {
    return jtag_error;
  }

  delayMicroseconds(us);

  jtag_error = jtag_pin_clear(pin);

  if (jtag_error != JTAG_NO_ERROR) {
    return jtag_error;
  }

  return JTAG_NO_ERROR;
}

int jtag_pulse_low(int pin, unsigned int us)
{
  int jtag_error;

  jtag_error = jtag_pin_clear(pin);
  if (jtag_error != JTAG_NO_ERROR) {
    return jtag_error;
  }

  delayMicroseconds(us);

  jtag_error = jtag_pin_set(pin);

  if (jtag_error != JTAG_NO_ERROR) {
    return jtag_error;
  }

  return JTAG_NO_ERROR;
}

int jtag_assign_pin(int jtag_pin, int digital_pin)
{
    int i_pin;

    if (jtag_pin >= N_JTAG_PINS || jtag_pin < 0) {
    return JTAG_ERROR_BAD_PIN;
    }

    if (digital_pin > 13 || digital_pin < 0) {
    return JTAG_ERROR_BAD_PIN;
    }

    for (i_pin = 0; i_pin < N_JTAG_PINS; i_pin++) {
    if (i_pin != jtag_pin && digital_pin == jtag_pin_map[i_pin]) {
        return JTAG_ERROR_BAD_PIN;
    }
    }

    jtag_pin_map[jtag_pin] = digital_pin;
    pinMode(digital_pin, jtag_pin_dir[jtag_pin]);

    return JTAG_NO_ERROR;
}

int jtag_set_speed(unsigned int khz)
{
  if (khz == 0 || khz > JTAG_MAX_SPEED_KHZ) {
    return JTAG_ERROR_BAD_SPEED;
  }

  // Mininum time for TCK to be stable is half the clock period.
  // For 100kHz of TCK frequency the period is 10us so jtag_min_tck_micros is
  // 5us.
  jtag_min_tck_micros = (500U + khz - 1) / khz; // ceil

  return JTAG_NO_ERROR;
}

/**
 * \brief Make one JTAG clock
 *
 * \param[in] tms: TMS bit state
 * \param[in] tdi: TDI bit state
 *
 * \return TDO state
 */
int jtag_clock(int tms, int tdi)
{
  unsigned long cur_micros;
  int tdo;

  // Setting TDI and TMS before rising edge of TCK.
  jtag_pin_write(TDI, tdi);
  jtag_pin_write(TMS, tms);

  Serial.print("TMS: ");
  Serial.println(tms);

  Serial.print("TDI: ");
  Serial.println(tdi);

  // Waiting until TCK has been stable for at least jtag_min_tck_micros.
  cur_micros = micros();

  if (cur_micros < jtag_last_tck_micros + jtag_min_tck_micros) {
    delayMicroseconds(jtag_last_tck_micros + jtag_min_tck_micros - cur_micros);
  }

//   tdo = jtag_pin_get(TDO); // TDO changes on falling edge of TCK, we are reading
//                            // value changed during last jtag_clock.

  jtag_pin_set(TCK);       // Rising edge of TCK. TDI and TMS are sampled in.
  delayMicroseconds(jtag_min_tck_micros);
  jtag_pin_clear(TCK);             // Falling edge of TCK. TDO is changing.

  jtag_last_tck_micros = micros(); // Saving timestamp of last TCK change.

  tdo = jtag_pin_get(TDO); // TDO changes on falling edge of TCK, we are reading
                          // value changed during last jtag_clock.

  return tdo;
}

/**
 * \brief Write JTAG sequence
 *
 * \param[in] n:  length of sequence in bits
 * \param[in] tms: array of TMS bits
 * \param[in] tdi: array of TDI bits
 * \param[out] tdo: array of TDO bits
 *
 * \return error code
 */
int jtag_sequence(unsigned int n, const byte *tms, const byte *tdi, byte *tdo)
{
  unsigned int i_seq;

  if (n > JTAG_MAX_SEQUENCE_LEN) {
    return JTAG_ERROR_BAD_SEQUENCE_LEN;
  }

  for (i_seq = 0; i_seq < n; i_seq++) {
    set_bit_in_array(i_seq, tdo, jtag_clock(get_bit_from_array(i_seq, tms), get_bit_from_array(i_seq, tdi)));
  }

  return JTAG_NO_ERROR;
}


/**
 * \brief
 *
 */
void jtag_reset(void)
{
  byte tms = 0x1F;
  byte tdi = 0x00;
  byte tdo = 0x00;

  jtag_sequence(5, &tms, &tdi, &tdo);
}

void jtag_ir(uint32_t length, byte* command, byte* output)
{
  byte tms_pre = 0x06;
  byte tms_post = 0x01;
  byte tdi_pre = 0x00;
  byte tdi_post = 0x00;
  int i_seq = 0;

  jtag_reset();

  jtag_sequence(5, &tms_pre, &tdi_pre, output);

  for (i_seq = 0; i_seq < length-1; i_seq++) {
    set_bit_in_array(i_seq, output, jtag_clock(0, get_bit_from_array(i_seq, command)));
  }
  // for (size_t i = 0; i < length-1; i++)
  // {
  //   Serial.print(get_bit_from_array(i, output));
  // }

  set_bit_in_array(i_seq, output, jtag_clock(1, get_bit_from_array(i_seq, command)));
  // for (size_t i = 0; i < 1; i++)
  // {
  //   Serial.print(get_bit_from_array(i, output));
  // }


  jtag_sequence(2, &tms_post, &tdi_post, output);
}

void jtag_dr(uint32_t length, byte* data, byte* output)
{
  byte tms_pre = 0x02;
  byte tms_post = 0x01;
  byte tdi_pre = 0x00;
  byte tdi_post = 0x00;
  int i_seq = 0;

  jtag_reset();

  jtag_sequence(4, &tms_pre, &tdi_pre, output);
  for (size_t i = 0; i < 1; i++)
  {
    Serial.print(get_bit_from_array(i, &output[2]));
  }

  for (i_seq = 0; i_seq < length-1; i_seq++) {
    set_bit_in_array(i_seq, output, jtag_clock(0, get_bit_from_array(i_seq, data)));
  }
  for (size_t i = 0; i < length-1; i++)
  {
    Serial.print(get_bit_from_array(i, output));
  }

  set_bit_in_array(i_seq, output, jtag_clock(1, get_bit_from_array(i_seq, data)));
  for (size_t i = 0; i < 1; i++)
  {
    Serial.print(get_bit_from_array(i, output));
  }


  jtag_sequence(2, &tms_post, &tdi_post, output);
  // for (size_t i = 0; i < 2; i++)
  // {
  //   Serial.print(get_bit_from_array(i, output));
  // }

  Serial.println("");
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("JTAG setup");
    jtag_setup();

    // Serial.println("JTAG setup");
    jtag_set_speed(800000);

    Serial.println("Start");
}

void loop()
{
  byte data[33] = {0};
  byte output[33] = {0};

  byte command[] = {0xFA, 0x01};
  jtag_ir(9, command, output);

  Serial.println("================================================");

  // jtag_dr(sizeof(data), data, output);

    delay(1000);
}
