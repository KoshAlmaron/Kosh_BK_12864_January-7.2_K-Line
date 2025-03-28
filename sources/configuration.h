//=============================================================================
//=============================== ЭКРАН =======================================
//=============================================================================

// SSD1309 - 0
// SSD1306 - 1

#define OLED_TYPE 0

//=============================================================================
//============================= Настройки =====================================
//=============================================================================

// 25959 импульсов на км.
#define IMPULSES_PER_METER 26		// Число импульсов датчика скорости на 1 м.
#define IMPULSES_PER_MLITER 16		// Число импульсов расхода топлива на 1 мл.

#define	LCD_UPDATE_PERIOD 200		// Период одновления экрана.
#define ANSWER_WAIT_TIME 1000		// Время ожидания ответа.
#define COMM_RESEND_DELAY 1000		// Задержка перед повторной отправкой команды.

// Номер последнего экрана.
#define LCD_LAST_DISPLAY 6

// Скорость анимаций.
#define CHANGE_ANIMATION_RPC 7	// Смена экрана, cтрок за цикл.
#define CHANGE_ANIMATION_MS 20	// Смена экрана, мс цикл.
#define START_ANIMATION_MS 40	// Начала/завершение работы, мс цикл.

#define LCD_BRIGHT_MIN 0		// Минимальная яркость экрана.
#define LCD_BRIGHT_MAX 200		// Максмальная яркость экрана.

#define LCD_BRIGHT_STEP 16		// Шаг изменения яркости.
#define BRIGHT_BOX_TIMER 3000	// Время отображения блока настройки яркости.
#define END_SCREEN_TIMER 8000	// Время отображения экрана завершения.

#define SPEED_CHIME_LIMIT 100		// Лимит скорости
#define SPEED_CHIME_INTERVAL 1000	// Базовый интервал включени колокольчика.
#define SPEED_CHIME_DELAY 380		// Время удержания колокольчика.

//=============================================================================
//============ Лимиты показаний для срабатывания сигнализации =================
//=============================================================================

// Пороги значений для температуры ОЖ
#define WATER_TEMP_MIN -30
#define WATER_TEMP_MAX 96

// Порог значений для ШДК
#define LAMBDA_AFR_MIN 120	// x10
#define LAMBDA_AFR_MAX 152	// x10

// Порог значений для УДК
#define UDK_VOLT_MIN 1		// x100
#define UDK_VOLT_MAX 87		// x100

// Порог значений оборотов
#define RPM_LIMIT 5500

// Пороги значений для лямбда коррекции
#define LAMBDA_CORR_MIN (-10 * 256)
#define LAMBDA_CORR_MAX (10 * 256)

// Пороги напряжения сети
#define BATT_VOLT_MIN 120	// x10
#define BATT_VOLT_MAX 145	// x10

// Пороги значений для температуры воздуха
#define AIR_TEMP_MIN -20
#define AIR_TEMP_MAX 50

// Порог значений наддува
#define OVERBOOST_LIMIT 176

// Порог значений давления масла
#define OIL_PRESSURE_MIN 10	// x10
#define OIL_PRESSURE_MAX 55	// x10

//=============================================================================
//================================== Отладка ==================================
//=============================================================================
// Режим отладки. Проверка работы экранов без подключения к SECU.
// Раскомментировать для включения.
#define DEBUG_MODE

// Запись значений пробега и расхода, если случайно затер или при замене платы.
// Расход топлива храниться в мл, потому надо умножать литры на 1000.

// Для записи в EEPROM указанных ниже значений при запуске БК держать кнопку "Влево".

// При варианте с энкодером раскомментировать строку ниже, прошить,
// закомментировать и прошить.
//#define WRITE_EEPROM_ON_START

// 30.06.2024
// Раскомментировать параметры, которые необходимо записать в EEPROM	 
#define WRITE_DISTANCE_DAY (0 * 1000LU)
#define WRITE_DISTANCE_ALL (27793 * 1000LU)
#define WRITE_FUEL_BURNED_DAY (0 * 1000LU)
#define WRITE_FUEL_BURNED_ALL (2451 * 1000LU)

#define WRITE_BRIGHT_LCD_NIGHT 0
#define WRITE_BRIGHT_LCD_DAY 255