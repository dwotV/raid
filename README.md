# raid

## Ejecutar
### Crear paridad
```
./raid create <archivo1> <archivo2>
```
### Recuperar archivo
```
./raid recover <archivo_existente.txt> <parity.bin> <archivo_a_recuperar.txt>
```
## Simular Error
```
printf "\xFF" | dd of=<archivo> bs=1 seek=100 count=1 conv=notrunc
```
