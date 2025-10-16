#pragma once

#define S2WS(str) std::wstring(str.begin(), str.end())

#include <vector>
#include <string>

using buffer_t    = std::vector<int8_t>;

// Структура отказов
struct fault_t
{
	uint8_t value;

	union
	{
		uint8_t time_out : 1;
	} bits;
};

typedef union
{
	uint32_t value;
	struct
	{
		uint32_t label  : 8;  // Бит 0-7: Значение метки 
		uint32_t sdi    : 2;  // Бит 8-9: ID источника/получателя
		uint32_t data   : 19; // Бит 10-28: Данные
		uint32_t ssm    : 2;  // Бит 29-30: Матрица SSM
		uint32_t parity : 1;  // Бит 31: Бит чётности
	} bits;
} a429_word_t;


/*
* PANEL – регистр пульта управления топливной системой
* Смещение: + 00h
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t lh_ac_aux_pump_cmd : 1;  //Бит 0: Команда включения левого дополнительного насоса
        uint32_t lh_ac_main_pump_cmd : 1; //Бит 1: Команда включения левого основного насоса
        uint32_t lh_addt_pump_cmd : 1;    //Бит 2: Команда включения левого дополнительного насоса
        uint32_t rh_ac_aux_pump_cmd : 1;  //Бит 3: Команда включения правого дополнительного насоса
        uint32_t rh_ac_main_pump_cmd : 1; //Бит 4: Команда включения правого основного насоса
        uint32_t rh_addt_pump_cmd : 1;    //Бит 5: Команда включения правого дополнительного насоса
        uint32_t xfeed_on_ch1 : 1;        //Бит 6: Команда включения перекрестного питания канал 1
        uint32_t xfeed_on_ch2 : 1;        //Бит 7: Команда включения перекрестного питания канал 2
        uint32_t xfeed_off_ch1 : 1;       //Бит 8: Команда выключения перекрестного питания канал 1
        uint32_t xfeed_off_ch2 : 1;       //Бит 9: Команда выключения перекрестного питания канал 2
        uint32_t reserved : 22;           //Бит 10-31: Зарезервировано
    } bits;
} PANEL_REG_t;

/*
* VALVES_IN – регистр состояния запорных кранов (входные сигналы)
* Смещение: + 04h
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t apu_vlv_closed : 1;   //Бит 0: Кран ВСУ закрыт
        uint32_t apu_vlv_open : 1;     //Бит 1: Кран ВСУ открыт
        uint32_t ct_rv_open : 1;       //Бит 2: Кран центрального бака открыт
        uint32_t ct_rv_closed : 1;     //Бит 3: Кран центрального бака закрыт
        uint32_t dv_open : 1;          //Бит 4: Дренажный клапан открыт
        uint32_t dv_closed : 1;        //Бит 5: Дренажный клапан закрыт
        uint32_t l_eng_vlv_closed : 1; //Бит 6: Кран левого двигателя закрыт
        uint32_t l_eng_vlv_open : 1;   //Бит 7: Кран левого двигателя открыт
        uint32_t lh_rv_open : 1;       //Бит 8: Кран левого бака открыт
        uint32_t lh_rv_closed : 1;     //Бит 9: Кран левого бака закрыт
        uint32_t r_eng_vlv_closed : 1; //Бит 10: Кран правого двигателя закрыт
        uint32_t r_eng_vlv_open : 1;   //Бит 11: Кран правого двигателя открыт
        uint32_t rh_rv_open : 1;       //Бит 12: Кран правого бака открыт
        uint32_t rh_rv_closed : 1;     //Бит 13: Кран правого бака закрыт
        uint32_t xfeed_vlv_closed : 1; //Бит 14: Кран перекрестного питания закрыт
        uint32_t xfeed_vlv_open : 1;   //Бит 15: Кран перекрестного питания открыт
        uint32_t reserved : 16;        //Бит 16-31: Зарезервировано
    } bits;
} VALVES_IN_REG_t;

/*
* PUMPS_IN – регистр состояния топливных насосов (входные сигналы)
* Смещение: + 08h
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t lh_addt_pump_normal_press : 1;    //Бит 0: Левый дополнительный насос - нормальное давление
        uint32_t lh_addt_pump_low_press : 1;       //Бит 1: Левый дополнительный насос - низкое давление
        uint32_t rh_addt_pump_normal_press : 1;    //Бит 2: Правый дополнительный насос - нормальное давление
        uint32_t rh_addt_pump_low_press : 1;       //Бит 3: Правый дополнительный насос - низкое давление
        uint32_t lh_ac_main_pump_normal_press : 1; //Бит 4: Левый основной насос - нормальное давление
        uint32_t lh_ac_main_pump_low_press : 1;    //Бит 5: Левый основной насос - низкое давление
        uint32_t lh_ac_aux_pump_normal_press : 1;  //Бит 6: Левый вспомогательный насос - нормальное давление
        uint32_t lh_ac_aux_pump_low_press : 1;     //Бит 7: Левый вспомогательный насос - низкое давление
        uint32_t rh_ac_main_pump_normal_press : 1; //Бит 8: Правый основной насос - нормальное давление
        uint32_t rh_ac_main_pump_low_press : 1;    //Бит 9: Правый основной насос - низкое давление
        uint32_t rh_ac_aux_pump_normal_press : 1;  //Бит 10: Правый вспомогательный насос - нормальное давление
        uint32_t rh_ac_aux_pump_low_press : 1;     //Бит 11: Правый вспомогательный насос - низкое давление
        uint32_t lh_addt_pump_powered : 1;         //Бит 12: Левый дополнительный насос под напряжением
        uint32_t rh_addt_pump_powered : 1;         //Бит 13: Правый дополнительный насос под напряжением
        uint32_t lh_ac_main_pump_powered : 1;      //Бит 14: Левый основной насос под напряжением
        uint32_t lh_ac_aux_pump_powered : 1;       //Бит 15: Левый вспомогательный насос под напряжением
        uint32_t rh_ac_main_pump_powered : 1;      //Бит 16: Правый основной насос под напряжением
        uint32_t rh_ac_aux_pump_powered : 1;       //Бит 17: Правый вспомогательный насос под напряжением
        uint32_t lh_eng_low_press : 1;             //Бит 18: Левый двигатель МСУ - низкое давление
        uint32_t lh_eng_norm_press : 1;            //Бит 19: Левый двигатель МСУ - нормальное давление
        uint32_t rh_eng_low_press : 1;             //Бит 20: Правый двигатель МСУ - низкое давление
        uint32_t rh_eng_norm_press : 1;            //Бит 21: Правый двигатель МСУ - нормальное давление
        uint32_t apu_low_press : 1;                //Бит 22: ВСУ - низкое давление
        uint32_t apu_norm_press : 1;               //Бит 23: ВСУ - нормальное давление
        uint32_t reserved : 8;                     //Бит 24-31: Зарезервировано
    } bits;
} PUMPS_IN_REG_t;

/*
* ENG_FIRE_IN – регистр пусков МСУ и пожарной защиты
* Смещение: + 0Ch
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t l_eng_off_ch1 : 1;  //Бит 0: Левый двигатель выключен канал 1
        uint32_t l_eng_off_ch2 : 1;  //Бит 1: Левый двигатель выключен канал 2
        uint32_t l_eng_on_ch1 : 1;   //Бит 2: Левый двигатель включен канал 1
        uint32_t l_eng_on_ch2 : 1;   //Бит 3: Левый двигатель включен канал 2
        uint32_t r_eng_off_ch1 : 1;  //Бит 4: Правый двигатель выключен канал 1
        uint32_t r_eng_off_ch2 : 1;  //Бит 5: Правый двигатель выключен канал 2
        uint32_t r_eng_on_ch1 : 1;   //Бит 6: Правый двигатель включен канал 1
        uint32_t r_eng_on_ch2 : 1;   //Бит 7: Правый двигатель включен канал 2
        uint32_t reserved : 24;      //Бит 8-31: Зарезервировано
    } bits;
} ENG_FIRE_IN_REG_t;

/*
* APU_IN – регистр пусков ВСУ и пожарной защиты
* Смещение: + 10h
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t apu_fire_ch1 : 1;  //Бит 0: Пожар ВСУ канал 1
        uint32_t apu_fire_ch2 : 1;  //Бит 1: Пожар ВСУ канал 2
        uint32_t apu_off_ch1 : 1;   //Бит 2: ВСУ выключена канал 1
        uint32_t apu_off_ch2 : 1;   //Бит 3: ВСУ выключена канал 2
        uint32_t apu_on_ch1 : 1;    //Бит 4: ВСУ включена канал 1
        uint32_t apu_on_ch2 : 1;    //Бит 5: ВСУ включена канал 2
        uint32_t reserved : 26;     //Бит 6-31: Зарезервировано
    } bits;
} APU_IN_REG_t;

/*
* VALVES_OUT – регистр управления запорными кранами (выходные сигналы)
* Смещение: + 00h
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t apu_vlv_close_cmd_ch1 : 1;      //Бит 0: Команда закрытия крана ВСУ канал 1
        uint32_t apu_vlv_close_cmd_ch2 : 1;      //Бит 1: Команда закрытия крана ВСУ канал 2
        uint32_t apu_vlv_open_cmd_ch1 : 1;       //Бит 2: Команда открытия крана ВСУ канал 1
        uint32_t apu_vlv_open_cmd_ch2 : 1;       //Бит 3: Команда открытия крана ВСУ канал 2
        uint32_t ct_refuel_vlv_open_cmd_ch1 : 1; //Бит 4: Команда открытия крана центрального бака канал 1
        uint32_t ct_refuel_vlv_open_cmd_ch2 : 1; //Бит 5: Команда открытия крана центрального бака канал 2
        uint32_t dv_closed_cmd : 1;              //Бит 6: Команда закрытия дренажного клапана
        uint32_t dv_open_cmd : 1;                //Бит 7: Команда открытия дренажного клапана
        uint32_t lh_refuel_vlv_open_cmd_ch1 : 1; //Бит 8: Команда открытия крана левого бака канал 1
        uint32_t lh_refuel_vlv_open_cmd_ch2 : 1; //Бит 9: Команда открытия крана левого бака канал 2
        uint32_t rh_refuel_vlv_open_cmd_ch1 : 1; //Бит 10: Команда открытия крана правого бака канал 1
        uint32_t rh_refuel_vlv_open_cmd_ch2 : 1; //Бит 11: Команда открытия крана правого бака канал 2
        uint32_t xfeed_vlv_close_cmd_ch1 : 1;    //Бит 12: Команда закрытия крана перекрестного питания канал 1
        uint32_t xfeed_vlv_close_cmd_ch2 : 1;    //Бит 13: Команда закрытия крана перекрестного питания канал 2
        uint32_t xfeed_vlv_open_cmd_ch1 : 1;     //Бит 14: Команда открытия крана перекрестного питания канал 1
        uint32_t xfeed_vlv_open_cmd_ch2 : 1;     //Бит 15: Команда открытия крана перекрестного питания канал 2
        uint32_t reserved : 16;                  //Бит 16-31: Зарезервировано
    } bits;
} VALVES_OUT_REG_t;

/*
* Структура входных регистров РК
*/
typedef struct
{
    PANEL_REG_t       PANEL;        // + 00h
    VALVES_IN_REG_t   VALVES_IN;    // + 04h
    PUMPS_IN_REG_t    PUMPS_IN;     // + 08h
    ENG_FIRE_IN_REG_t ENG_FIRE_IN;  // + 0Ch
    APU_IN_REG_t      APU_IN;       // + 10h
} RK_INPUT_REG_t;
