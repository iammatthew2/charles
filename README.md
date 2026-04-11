# Charles

Charles is a Seeed XIAO ESP32-C6 based animatronic eye controller. It receives commands from a paired remote (Daryl) over ESP-NOW and drives two servos: one for left/right pan of the eyeball and one for eyelid open/close.

Board power comes over USB. Servo power is external with a common ground.

## Remote control behavior (ESP-NOW)

Charles listens for `RemotePacket` frames broadcast by Daryl. The packet carries an encoder position, encoder delta, a buttons bitmask, and an encoder-press flag.

### Encoder
- Rotating the encoder pans the eyeball left/right (D0 servo).
- Boot default is **FAST** gain: 13 deg per encoder step.
- Button 5 (bit 4) toggles gain between **FAST** (13 deg/step) and **SLOW** (5 deg/step).
- Pressing the encoder recenters the pan servo to 90 deg.

### Buttons (bits 0–5)
| Bit | Action |
|-----|--------|
| 0   | Snap pan to 0 deg |
| 1   | Toggle eyelid open/closed |
| 2   | Snap pan to 90 deg |
| 3   | Snap pan to 135 deg |
| 4   | Toggle encoder gain (FAST ↔ SLOW) |
| 5   | Reserved |

### Sleep signal
When a packet arrives with bits 6 or 7 set (out-of-band, outside the valid `0b00111111` mask), Charles interprets it as Daryl entering deep sleep and enters **sleep pose**: pan centered at 90 deg, eyelid closed.

### Link loss
If no packet is received for 4 seconds, Charles enters the same **sleep pose** (pan centered, eyelid closed).

### MAC filter
`DARYL_MAC` in `main.cpp` can be set to Daryl's MAC address to reject packets from other senders. All zeros disables the filter (accept any sender).

## Wiring
 - left/right movement servo on pin D0
 - eye lid servo on pin D1

## Servos

Using Servo Motor Micro SG90
Product page: https://protosupplies.com/product/servo-motor-micro-sg90/

### Specs
Technical Specifications
Motor Model		Generic SG90 (China)
Drive Type		Analog
Degree Rotation		180° (±15°)
 Operating Ratings		
Voltage	4.8-6VDC  (5V Typical)
Current (idle)	10mA (typical)
Current (typical during movement)	100-250mA
Current (stall)	360mA (measured)
Stall Torque	1.7 kg-cm (measured)
Speed	0.12s / 60 degree (varies with VDC)
 Dimensions		
 Cable Length	 24cm  (9.5″)
 Motor Housing L x W x H	23 x 12 x 26mm (0.9 x0 .5 x  1″)
 Motor Height (w/ shaft)	32mm (1.26″)
Motor Housing Width with Mounting Ears	32mm (1.26″)