rm -rf temp_mtools
mkdir temp_mtools
cd temp_mtools
apt download mtools
mkdir mtools
cd mtools
ar x ../mtools*.deb
tar xf data.tar.zst
cp usr/bin/mtools ../../mcopy

cd ../..
rm -rf temp_mtools

echo "Ahora en el Makefile, modificar la linea que dice:"
echo "    MCOPY=mcopy"
echo "para que diga:"
echo "    MCOPY=./mcopy"
