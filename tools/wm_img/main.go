// wm_img - Genera la imagen de firmware (.img) y el archivo de carga (.fls)
// de WinnerMicro W80x a partir del .elf, igual que elf2bin.py + bin2img.py
// del WM IoT SDK, sin necesitar Python.
//
//	wm_img -elf sketch.elf -boot bootloader.img -pt partition_table.img \
//	       -max 0x1E0C00 -header 0x800F000 -run 0x800F400 -ver 2.4-rc \
//	       -flags 0x3 -img sketch.img -fls sketch.fls
//
// Parte de w80x_arduino (WM IoT SDK). LGPL 2.1.
package main

import (
	"bytes"
	"debug/elf"
	"encoding/binary"
	"flag"
	"fmt"
	"hash/crc32"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const (
	magic     = 0xA0FFFF9F
	flashBase = 0x08000000
	ckBase    = 0x08002000
)

func hexArg(s string) uint32 {
	v, err := strconv.ParseUint(s, 0, 32)
	if err != nil {
		fail("valor no valido: %s", s)
	}
	return uint32(v)
}

func fail(f string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, "wm_img: "+f+"\n", a...)
	os.Exit(1)
}

// elf2bin: .text + .rodata + .data (las secciones PROGBITS con direccion)
func elf2bin(path string, max uint32) []byte {
	f, err := elf.Open(path)
	if err != nil {
		fail("no se puede abrir %s: %v", path, err)
	}
	defer f.Close()
	var out bytes.Buffer
	for _, name := range []string{".text", ".rodata", ".data"} {
		s := f.Section(name)
		if s == nil || s.Type != elf.SHT_PROGBITS || s.Addr == 0 {
			continue
		}
		d, err := s.Data()
		if err != nil {
			fail("seccion %s: %v", name, err)
		}
		out.Write(d)
	}
	if uint32(out.Len()) > max {
		fail("el programa no cabe en la flash: %d bytes, maximo %d (sobran %d)",
			out.Len(), max, uint32(out.Len())-max)
	}
	return out.Bytes()
}

// bin2img: cabecera de 64 bytes + datos (alineados a 4 con 0xFF)
func bin2img(bin []byte, imgType uint16, header, run, update, next, updNo, flags uint32, ver string) []byte {
	for len(bin)%4 != 0 {
		bin = append(bin, 0xFF)
	}
	v := make([]byte, 16)
	copy(v, ver)
	if len(ver) >= 16 {
		v[15] = 0
	}
	var h bytes.Buffer
	w := func(x interface{}) { binary.Write(&h, binary.LittleEndian, x) }
	w(uint32(magic))
	w(imgType)
	w(uint16(0)) // sin compresion
	w(run)
	w(uint32(len(bin)))
	w(header)
	w(update)
	w(crc32.ChecksumIEEE(bin) ^ 0xFFFFFFFF)
	w(updNo)
	h.Write(v)
	w(flags)
	w(uint32(0))
	w(next)
	w(crc32.ChecksumIEEE(h.Bytes()) ^ 0xFFFFFFFF)
	return append(h.Bytes(), bin...)
}

func main() {
	if len(os.Args) > 1 && os.Args[1] == "cklink" {
		cklinkMain(os.Args[2:])
		return
	}
	if len(os.Args) > 1 && os.Args[1] == "serial" {
		serialMain(os.Args[2:])
		return
	}
	elfPath := flag.String("elf", "", "programa .elf")
	boot := flag.String("boot", "", "bootloader.img")
	pt := flag.String("pt", "", "partition_table.img")
	max := flag.String("max", "0x1E0C00", "tamano maximo del programa")
	header := flag.String("header", "0x800F000", "direccion de la cabecera de la imagen")
	run := flag.String("run", "0x800F400", "direccion de ejecucion")
	ver := flag.String("ver", "2.4-rc", "version (max 15 caracteres)")
	flags := flag.String("flags", "0x3", "bootloader config flags")
	imgOut := flag.String("img", "", "salida .img (opcional)")
	flsOut := flag.String("fls", "", "salida .fls (bootloader + programa + tabla de particiones)")
	ckOut := flag.String("ck", "", "salida .bin para CK-Link: imagen de la flash desde 0x08002000 (bootloader, tabla y programa)")
	sdk := flag.String("sdk", "", "carpeta sdk/<chip>: toma image.txt, img/bootloader.img e img/partition_table.img")
	flag.Parse()
	if *sdk != "" {
		set := map[string]*string{"max": max, "header": header, "run": run, "ver": ver, "flags": flags}
		d, err := os.ReadFile(filepath.Join(*sdk, "image.txt"))
		if err != nil {
			fail("no se encuentra %s", filepath.Join(*sdk, "image.txt"))
		}
		for _, l := range strings.Split(string(d), "\n") {
			kv := strings.SplitN(strings.TrimSpace(l), "=", 2)
			if len(kv) == 2 && set[kv[0]] != nil {
				*set[kv[0]] = kv[1]
			}
		}
		if *boot == "" {
			*boot = filepath.Join(*sdk, "img", "bootloader.img")
		}
		if *pt == "" {
			*pt = filepath.Join(*sdk, "img", "partition_table.img")
		}
	}
	if *elfPath == "" || (*imgOut == "" && *flsOut == "" && *ckOut == "") {
		flag.Usage()
		os.Exit(2)
	}
	bin := elf2bin(*elfPath, hexArg(*max))
	img := bin2img(bin, 1, hexArg(*header), hexArg(*run), 0, 0, 0, hexArg(*flags), *ver)
	if *imgOut != "" {
		if err := os.WriteFile(*imgOut, img, 0644); err != nil {
			fail("%v", err)
		}
	}
	if *ckOut != "" {
		bl, err := os.ReadFile(*boot)
		if err != nil {
			fail("no se encuentra %s", *boot)
		}
		ptd, err := os.ReadFile(*pt)
		if err != nil {
			fail("no se encuentra %s", *pt)
		}
		if err := os.WriteFile(*ckOut, flashImage([][]byte{bl, ptd, img}), 0644); err != nil {
			fail("%v", err)
		}
	}
	if *flsOut != "" {
		var fls bytes.Buffer
		for _, p := range []string{*boot} {
			d, err := os.ReadFile(p)
			if err != nil {
				fail("no se encuentra %s", p)
			}
			fls.Write(d)
		}
		fls.Write(img)
		d, err := os.ReadFile(*pt)
		if err != nil {
			fail("no se encuentra %s", *pt)
		}
		fls.Write(d)
		if err := os.WriteFile(*flsOut, fls.Bytes(), 0644); err != nil {
			fail("%v", err)
		}
	}
	fmt.Printf("Imagen: %d bytes de programa (maximo %d)\n", len(bin), hexArg(*max))
}

// flashImage arma el contenido de la flash a partir de las imagenes (.img):
// cabecera en su direccion (si esta en la flash) y datos en la de ejecucion.
// El resultado empieza en la direccion mas baja (0x08002000) y los huecos
// quedan a 0xFF. No toca los parametros de fabrica (0x08000000) ni NVS.
func flashImage(imgs [][]byte) []byte {
	type part struct {
		addr uint32
		data []byte
	}
	var parts []part
	for _, im := range imgs {
		if len(im) < 64 || binary.LittleEndian.Uint32(im) != magic {
			fail("imagen no valida")
		}
		run := binary.LittleEndian.Uint32(im[8:])
		n := binary.LittleEndian.Uint32(im[12:])
		hdr := binary.LittleEndian.Uint32(im[16:])
		if int(64+n) > len(im) {
			fail("imagen incompleta")
		}
		if hdr >= flashBase && hdr < flashBase+0x1000000 {
			parts = append(parts, part{hdr, im[:64]})
		}
		parts = append(parts, part{run, im[64 : 64+n]})
	}
	lo, hi := uint32(0xFFFFFFFF), uint32(0)
	for _, p := range parts {
		if p.addr < lo {
			lo = p.addr
		}
		if e := p.addr + uint32(len(p.data)); e > hi {
			hi = e
		}
	}
	out := bytes.Repeat([]byte{0xFF}, int(hi-lo))
	for _, p := range parts {
		copy(out[p.addr-lo:], p.data)
	}
	if lo != ckBase {
		fail("la imagen para CK-Link deberia empezar en 0x%08X y empieza en 0x%08X", ckBase, lo)
	}
	return out
}
