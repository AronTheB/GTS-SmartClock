
# GTS-Smart Clock

<img width="1920" height="1080" alt="Get To School (1920 x 1080 px)" src="https://github.com/user-attachments/assets/60ceec99-9153-4196-b8e2-1a3b472f665f" />

<br><br/>
For some reason I always need to run to the bus stop so I can catch the bus, so instead of trying to leave earlier I designed the GTS Smart Clock which can tell me when to leave so I reach it without running and waiting.

For my first non guided Hardware project i made a Smart Clock that can tell me the time, but it also connects to the public transport API and tells me when i my next bus going and also when will i get to my destination if i leave at that moment. Other features will be added in the future

The project uses a Xiao ESP32 C3, a OLED SSD1306 Screen, and a MxCherry switch with a keycap.

The clock is pretty simple to use. It has a single button which changes the current mode to the next one. The smart clock can be built by downloading the Gerber PCB files and buying them from a PCB printing company like JLCPCB or PCBWAY. You also need to buy the correct components which are linked in the BOM section, and 3d print the case or design a new one. After everything arrives solder everything to the board and upload the firmware to the ESP32. After that just put everything in the case like in the CAD model and connect it to power via a c-type cable. You can close the case by mainly screwing it in the screwhole with a 2mm x 5mm screw, but you could make it work with saller or a bit larger one (If your 3d printer cant print the icons on the top just remove them from the model and print it without them)

You need to also set up a config.h file in the included folder. (you might have to rewrite the api part of the code if you are using a different api)

#define WIFI_SSID     "WIFINAME"
#define WIFI_PASSWORD "WIFIPASSPORT"
#define BKK_API_KEY   "PUBLICTRANSPORTAPIKEY"
#define BKK_STOP_ID    "BUSSTOPID"
#define BUS_ROUTE_SHORT_NAME "BUSNAME"
#define BUS_DIRECTION_HEADSIGN "BUSDESTINATION"

In the pcb the pins for the oled screen are reveresd so you need to use wires to wire them in instead of a header pin

## KIKAD
This is the Schematic:

<img width="805" height="385" alt="image" src="https://github.com/user-attachments/assets/6234416e-a4b6-49ac-b5cf-0e0799ae2202" />


<br></br>

This is the PCB layout and wiring:

<img width="1264" height="716" alt="image" src="https://github.com/user-attachments/assets/b0e8ef6a-a831-445a-946b-4347c4431aab" />

<br></br>
## FUSION
This is the 3D Model of the case:

<img width="585" height="546" alt="Screenshot 2026-04-18 154616" src="https://github.com/user-attachments/assets/130c4cb8-347b-432d-aa5a-70d6fbaa6ea4" />



## BOM

| Name               | Link        | Price  | Quantity | Need To Buy? |
|--------------------|------------|--------|----------|--------------|
| ESP32 DevKitC      | [LINK](https://www.aliexpress.com/item/1005007084388623.html?spm=a2g0o.detail.0.0.17fcu9H1u9H1Gp&mp=1#nav-review) | $5.13  | 1 | Yes |
| SSD1306            | [LINK](https://www.aliexpress.com/item/1005006141235306.html?spm=a2g0o.productlist.0.0.1ec2tPXFtPXFrn&mp=1) | $1.00  | 1 | Yes |
| Cherry MX Switch   | [LINK](https://www.aliexpress.com/item/1005008586371182.html?spm=a2g0o.productlist.main.5.1ec2tPXFtPXFrn&algo_pvid=3f7b64de-8767-4574-abf2-40f9073028dd&pdp_ext_f=%7B%22order%22%3A%22130%22%2C%22spu_best_type%22%3A%22price%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005008586371182%7C_p_origin_prod%3A) | $2.09  | 1 | No  |
| Cherry MX Keycaps  | [LINK](https://www.aliexpress.com/item/1005002906017844.html?spm=a2g0o.productlist.main.2.7587d75duLmAxW&algo_pvid=7f1486d7-8f33-438b-a090-75c0d12eb0bc&pdp_ext_f=%7B%22order%22%3A%22578%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005002906017844%7C_p_origin_prod%3A) | $1.00  | 1 | No  |
| 3D Case            | Printing Legion     | Free + Shipping | 1 | Yes |
| PCB                | JLCPCB     | ≈ $5.00 | 1 | Yes |

**Total Cost:** $14.22



