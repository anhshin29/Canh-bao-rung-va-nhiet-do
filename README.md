# Hệ thống giám sát nhiệt độ, độ rung và điều khiển quạt sử dụng STM32 và FreeRTOS

**STM32F103C8 | C | FreeRTOS | PCF8591 | Software I2C | PWM | Keil uVision**

## 1. Giới thiệu

Dự án xây dựng một hệ thống nhúng sử dụng **STM32F103C8** để thu thập tín hiệu nhiệt độ và độ rung thông qua bộ chuyển đổi ADC **PCF8591**. Dữ liệu cảm biến được xử lý bởi hai task FreeRTOS, trao đổi với nhau bằng **Queue**, sau đó dùng để điều khiển **quạt PWM** và **LED cảnh báo**.

Phần mềm được phát triển bằng ngôn ngữ C trên **Keil uVision**, sử dụng **STM32 Standard Peripheral Library (StdPeriph)**. Giao tiếp với PCF8591 được triển khai bằng **Software I2C** thông qua GPIO, thay vì sử dụng ngoại vi I2C phần cứng.

## 2. Chức năng chính

- **Đọc nhiệt độ:** Sử dụng PCF8591, kênh ADC0 (AIN0). Giá trị ADC 8-bit được quy đổi bằng công thức hiện có trong mã nguồn: `temperature = adc_value * 500.0 / 255.0`.
- **Đánh giá độ rung:** Đọc 30 mẫu trên kênh ADC1 (AIN1), tính mức biến thiên `max - min` để nhận biết tín hiệu rung.
- **Cảnh báo bằng LED:** Khi mức biến thiên độ rung lớn hơn `10`, chương trình kích hoạt chuỗi nhấp nháy LED trên chân PA5.
- **Điều khiển quạt:** Khi nhiệt độ dưới `30.0`, PWM đặt về `0%`; từ `30.0` trở lên, PWM được đặt ở `100%`.
- **Đa nhiệm thời gian thực:** Một task đọc cảm biến và một task xử lý điều khiển, truyền dữ liệu qua Queue FreeRTOS.
- **Theo dõi khi debug:** Hai biến `DEBUG_NhietDo` và `DEBUG_DoRung` lưu dữ liệu mới nhận để quan sát trong trình gỡ lỗi.

> **Lưu ý:** Công thức quy đổi nhiệt độ phụ thuộc vào loại cảm biến, điện áp tham chiếu và cách ghép nối thực tế. Mã nguồn chưa chứa phần hiệu chuẩn hoặc xác định chính xác model cảm biến.

## 3. Kiến trúc hệ thống

```text
Cảm biến nhiệt độ (analog) ──> PCF8591 AIN0 ──┐
                                               ├─> Software I2C (PB6/PB7)
Cảm biến độ rung (analog) ───> PCF8591 AIN1 ──┘             |
                                                            v
                                                  Sensor_Task (Priority 1)
                                                            |
                                              FreeRTOS Queue (5 phần tử)
                                                            |
                                                            v
                                                  Control_Task (Priority 2)
                                                      /             \
                                                     v               v
                                             LED cảnh báo PA5     Quạt PWM PA8
```

### Hai task FreeRTOS

| Task | Ưu tiên | Chức năng |
| --- | ---: | --- |
| `Sensor_Task` | 1 | Đọc nhiệt độ và độ rung, đóng gói dữ liệu và gửi vào Queue; sử dụng `vTaskDelay(100 ms)` sau mỗi lần lấy mẫu. |
| `Control_Task` | 2 | Chờ nhận dữ liệu từ Queue bằng `xQueueReceive(..., portMAX_DELAY)`, sau đó điều khiển LED và quạt theo ngưỡng. |

Dữ liệu trao đổi được định nghĩa bởi `SensorData_t`, gồm `float NhietDo` và `uint8_t DoRung`. Queue được tạo với dung lượng **5 phần tử**.

## 4. Kết nối phần cứng

| Chức năng | Chân STM32 | Ghi chú |
| --- | --- | --- |
| I2C SCL | PB6 | GPIO open-drain, Software I2C |
| I2C SDA | PB7 | GPIO open-drain, Software I2C |
| LED cảnh báo | PA5 | GPIO output push-pull |
| Điều khiển quạt | PA8 | TIM1 Channel 1, đầu ra PWM |
| Tín hiệu nhiệt độ | PCF8591 AIN0 | Đầu vào ADC kênh 0 |
| Tín hiệu độ rung | PCF8591 AIN1 | Đầu vào ADC kênh 1 |

**Địa chỉ PCF8591:** `0x48` ở định dạng địa chỉ 7-bit, tương ứng byte địa chỉ ghi `0x90` trong code.

**Lưu ý phần cứng:** Bus I2C open-drain cần điện trở kéo lên phù hợp với mức điện áp của mạch. Chân PA8 cung cấp tín hiệu điều khiển PWM; quạt cần mạch công suất/driver phù hợp, không cấp nguồn cho quạt trực tiếp từ GPIO.

## 5. Nguyên lý hoạt động

1. Hàm `main()` khởi tạo Software I2C, chân GPIO đầu ra và TIM1 PWM.
2. FreeRTOS tạo `xDataQueue` gồm 5 phần tử, sau đó khởi tạo `Sensor_Task` và `Control_Task`.
3. `Sensor_Task` đọc ADC0 để tính nhiệt độ, đồng thời lấy 30 mẫu ADC1 để tính độ biến thiên rung (`max - min`).
4. Các giá trị được gửi qua Queue sang `Control_Task`.
5. `Control_Task` nhận dữ liệu, cập nhật biến debug, xử lý cảnh báo LED và điều khiển PWM quạt.

### Ngưỡng điều khiển trong mã nguồn

| Điều kiện | Kết quả |
| --- | --- |
| `NhietDo < 30.0` | `Fan_Control(0)` |
| `NhietDo >= 30.0` | `Fan_Control(100)` |
| `DoRung > 10` | Đặt `alarm_timer = 10`; LED đảo trạng thái qua các lượt xử lý tiếp theo |
| `alarm_timer == 0` | LED tắt |

Hàm `Fan_Control()` hỗ trợ mức PWM từ 0 đến 100, nhưng **logic điều khiển hiện tại chỉ sử dụng hai mức 0 và 100**. Với giả thiết xung clock TIM1 là 72 MHz, bộ chia `72 - 1` và chu kỳ đếm `100 - 1` tạo tần số PWM khoảng **10 kHz**.

## 6. Cấu trúc mã nguồn

```text
ADC/
├── ADC.uvprojx          # File dự án Keil uVision
├── main.c               # Khởi tạo hệ thống, task và FreeRTOS Queue
├── pcf8591.c            # Software I2C, đọc ADC và ghi DAC PCF8591
├── pcf8591.h            # Cấu hình chân I2C và khai báo hàm PCF8591
├── sensor.c             # Xử lý giá trị nhiệt độ và độ rung
├── sensor.h             # Khai báo các hàm cảm biến
├── output.c             # Khởi tạo GPIO, TIM1 PWM và điều khiển LED/quạt
├── output.h             # Khai báo GPIO LED và các hàm đầu ra
└── README.md            # Tài liệu dự án
```

Các file `ADC.uvoptx`, `ADC.uvguix.*` là cấu hình và dữ liệu giao diện/debug của Keil, không phải phần xử lý chính của chương trình.

## 7. Công cụ và thư viện

- **Vi điều khiển:** STM32F103C8 (ARM Cortex-M3).
- **Ngôn ngữ:** Embedded C.
- **IDE:** Keil MDK/uVision; file dự án cấu hình Arm Compiler 6.
- **RTOS:** FreeRTOS; cấu hình project tham chiếu gói CMSIS-FreeRTOS 10.2.0.
- **Driver:** STM32 Standard Peripheral Library (StdPeriph), CMSIS.
- **Giao tiếp và ngoại vi:** Software I2C, GPIO, Timer PWM, ADC ngoài PCF8591.

## 8. Hướng dẫn mở dự án

1. Cài đặt Keil MDK/uVision và bộ hỗ trợ thiết bị **STM32F1xx_DFP**.
2. Cài các thành phần **CMSIS**, **CMSIS-FreeRTOS** và **StdPeriph** tương ứng với cấu hình của project.
3. Mở file `ADC.uvprojx`, kiểm tra target `STM32F103C8`.
4. Bảo đảm các file trong thư mục `RTE/` (đặc biệt là `FreeRTOSConfig.h`, tệp startup và cấu hình StdPeriph) tồn tại hoặc được tạo lại từ các pack.
5. Kiểm tra clock, sơ đồ nối dây, điện áp tham chiếu ADC và ngưỡng cảm biến trước khi build và nạp chương trình.
6. Build trong Keil; kết nối bộ nạp/debug phù hợp để chạy thử trên phần cứng.

**Trạng thái bộ file được chia sẻ:** Các file `RTE/` không nằm trong bộ file đã cung cấp, nên chưa thể xác nhận toàn bộ dự án có thể build độc lập chỉ từ các file này. README mô tả mã nguồn đã được đọc, không khẳng định đã thử nghiệm thành công trên phần cứng.

## 9. Giới hạn và hướng cải tiến

- Kiểm chứng công thức quy đổi nhiệt độ theo cảm biến và điện áp tham chiếu thực tế.
- Bổ sung kiểm tra ACK, xử lý lỗi giao tiếp I2C và cơ chế timeout phù hợp.
- Kiểm tra kết quả tạo từng task và trạng thái gửi Queue để phát hiện lỗi tài nguyên.
- Bổ sung lọc nhiễu, hysteresis và hiệu chỉnh ngưỡng nhiệt độ/độ rung.
- Mở rộng điều khiển tốc độ quạt theo nhiều mức PWM thay vì chỉ bật/tắt.
- Đóng gói đầy đủ cấu hình `RTE/`, sơ đồ kết nối và hình ảnh phần cứng để người khác dễ tái hiện dự án.

## 10. Mục đích dự án

Dự án tập trung thực hành **lập trình STM32 bằng StdPeriph**, **triển khai Software I2C**, **đọc ADC ngoài PCF8591**, **thiết kế tác vụ FreeRTOS**, **truyền dữ liệu bằng Queue** và **điều khiển PWM/LED theo ngưỡng cảm biến**.
