# Baseus AeQur GH02 dongle: no sound on Linux (fix)

On Linux, the Baseus AeQur GH02 headset plays no sound through its 2.4G USB dongle. The microphone works, the system says audio is playing, and the headset stays silent. On Windows and macOS the same dongle works.

This repo has a small kernel module that fixes it. It installs through DKMS, so it is rebuilt automatically for every new kernel.

## Is this your problem?

```sh
lsusb | grep 10d6:4802                                   # Actions "USB Audio & HID"
lsusb -v -d 10d6:4802 2>/dev/null | grep -E "bNumInterfaces|wTotalLength" | head -2
```

If the dongle reports **4 interfaces / `wTotalLength 0x0105` (261)**, you are hitting this bug. After the fix it reports **6 interfaces / 291 bytes**.

## Why it happens

The dongle has two audio outputs:

- **"usb audio voice"**, which never reaches the headset, on any OS;
- **"usb audio game"** (endpoint `0x02`), which is the one that actually plays.

Windows and macOS receive a configuration descriptor that lists both outputs. For a reason that is not visible on the wire, Linux receives a reduced descriptor (4 interfaces) that leaves out the game output, so the only output Linux can use is a dead one.

Changing how Linux enumerates the device does not change which descriptor it gets. That covers enumeration schemes, timing, resets, hubs and quirks. The firmware still serves the game output when the host asks for it, though.

The module adds a kretprobe on `usb_get_descriptor()`. For this device only (`10d6:4802`, configuration 0, and only when the dongle reports the 261-byte descriptor), it hands the USB core the 291-byte descriptor the dongle gives to Windows (captured with USBPcap). Linux then sees and uses the game output. Nothing is written to the dongle.

## Install

Requirements: `dkms`, a compiler, and the headers of your running kernel.

```sh
sudo apt install dkms build-essential linux-headers-$(uname -r)   # Debian/Ubuntu
git clone https://github.com/ale-janicki/baseus-gh02-linux.git
cd baseus-gh02-linux
sudo sh install.sh
```

`install.sh`:

1. registers the module with DKMS (`/usr/src/gh02-fix-1.0`) and builds it for every installed kernel that has headers;
2. loads it at boot (`/etc/modules-load.d/gh02-fix.conf`);
3. installs `gh02-fix.service`, which power-cycles the dongle's USB port once at boot if the dongle came up before the module loaded;
4. loads the module now and re-enumerates the dongle.

**Secure Boot:** DKMS signs the module with your MOK key. If Secure Boot is on and the key is not enrolled, the module will not load. See `mokutil --import`.

## Select the game output

With PipeWire (most current distros):

```sh
wpctl status    # find the "USB Audio & HID" device id
wpctl set-profile <device-id> <index of "Pro Audio">
```

Then pick **"USB Audio & HID Pro 1"** as the output and **"USB Audio & HID Pro"** as the input, in your sound settings or with `wpctl set-default <id>`. "Pro 1" is the game output. Your desktop remembers the choice.

The "Digital Stereo (IEC958)" and "Analog Stereo" profiles still open the voice output, which is silent.

## Uninstall

```sh
sudo sh uninstall.sh
```

## Tested

- Ubuntu 26.04, kernel 7.0.0-43-generic (also built on 7.0.0-42 and 7.0.14), PipeWire.
- Headset firmware V1.0.14 (Baseus app).

The module only acts on a dongle that reports exactly the 261-byte descriptor. If your firmware is different it does nothing. In that case, please open an issue with the output of `lsusb -v -d 10d6:4802`.

## Files

| File | What it is |
|---|---|
| `gh02_fix.c` | the kernel module (kretprobe on `usb_get_descriptor`) |
| `gh02_cfg291.h` | the 291-byte configuration descriptor the dongle sends to Windows |
| `gh02-reenumerate`, `gh02-fix.service` | re-enumerate the dongle at boot if needed |
| `dkms.conf`, `Makefile` | DKMS build |
| `install.sh`, `uninstall.sh` | setup and removal |

## License

GPL-2.0. See `LICENSE`.

Not affiliated with Baseus. Use at your own risk.
