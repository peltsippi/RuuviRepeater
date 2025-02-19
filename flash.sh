args=("$@")
if [ -z "${args[0]}" ]; then
    echo "Usage: ./flash.sh PARTICLEBOARDNAMEHERE"
fi
particle compile ${args[0]} ./src/RuuviRepeater.cpp --saveTo RuuviRepeater.bin && particle usb dfu && particle flash --usb RuuviRepeater.bin
