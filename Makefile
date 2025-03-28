# Бортовой компьютер c графическим дисплеем 128х64 на Arduino "Сделай сам" версия для K-Line  (j79sx)

# Название проекта
PROJECT = Kosh_BK_12864_K-Line_v1

# Чип и скорость записи
DEVICE = atmega328p
F_CPU = 16000000
BAUD = 115200
PORT = /dev/ttyACM0

# Комманды и опции компиляции
COMPILE = avr-gcc -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRDUDE = avrdude -p $(DEVICE) -D -c arduino -P $(PORT) -b $(BAUD)

# Список файлов
SRC = $(wildcard sources/*.c)
OBJ = $(patsubst sources/%.c, output/%.o, $(SRC))

# Список целей - не файлов для исключения
.PHONY: all flash clean

# Цель по-умолчанию, требует наличия файла .bin
all: $(PROJECT).bin

# Создание файлов .bin, требует наличие фалов .o
$(PROJECT).bin: $(OBJ)
	$(COMPILE) -o output/$(PROJECT).elf $(OBJ)
	$(OBJCOPY) -O binary output/$(PROJECT).elf output/$(PROJECT).bin
	$(OBJDUMP) -Pmem-usage output/$(PROJECT).elf
	rm -f output/*.o
	rm -f output/*.elf

# Создание файлов .o, требует наличие фалов .c
output/%.o : sources/%.c 
	$(COMPILE) -c $< -o $@

# Запись прошивки
flash:	$(PROJECT).bin
	$(AVRDUDE) -U flash:w:output/$(PROJECT).bin:r

# Очистка рабочей папки
clean:
	rm -f output/*.*
	