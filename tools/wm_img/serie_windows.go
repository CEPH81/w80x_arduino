//go:build windows

// serie_windows.go - puerto serie minimo (115200 8N1, sin control de flujo) para Windows.
package main

import (
	"strings"
	"syscall"
	"unsafe"
)

var (
	k32                 = syscall.NewLazyDLL("kernel32.dll")
	pGetCommState       = k32.NewProc("GetCommState")
	pSetCommState       = k32.NewProc("SetCommState")
	pSetCommTimeouts    = k32.NewProc("SetCommTimeouts")
	pEscapeCommFunction = k32.NewProc("EscapeCommFunction")
)

type dcb struct {
	DCBlength  uint32
	BaudRate   uint32
	Flags      uint32
	wReserved  uint16
	XonLim     uint16
	XoffLim    uint16
	ByteSize   byte
	Parity     byte
	StopBits   byte
	XonChar    byte
	XoffChar   byte
	ErrorChar  byte
	EofChar    byte
	EvtChar    byte
	wReserved1 uint16
}

type commTimeouts struct {
	ReadIntervalTimeout, ReadTotalTimeoutMultiplier, ReadTotalTimeoutConstant uint32
	WriteTotalTimeoutMultiplier, WriteTotalTimeoutConstant                    uint32
}

type puerto struct{ h syscall.Handle }

func abrirPuerto(nombre string) (*puerto, error) {
	if !strings.HasPrefix(nombre, `\\.\`) {
		nombre = `\\.\` + nombre // necesario para COM10 y siguientes
	}
	n, err := syscall.UTF16PtrFromString(nombre)
	if err != nil {
		return nil, err
	}
	h, err := syscall.CreateFile(n, syscall.GENERIC_READ|syscall.GENERIC_WRITE, 0, nil, syscall.OPEN_EXISTING, 0, 0)
	if err != nil {
		return nil, err
	}
	var d dcb
	d.DCBlength = uint32(unsafe.Sizeof(d))
	if r, _, e := pGetCommState.Call(uintptr(h), uintptr(unsafe.Pointer(&d))); r == 0 {
		syscall.CloseHandle(h)
		return nil, e
	}
	d.BaudRate = 115200
	d.Flags = 1 // fBinary; DTR y RTS desactivados, sin control de flujo
	d.ByteSize = 8
	d.Parity = 0
	d.StopBits = 0
	if r, _, e := pSetCommState.Call(uintptr(h), uintptr(unsafe.Pointer(&d))); r == 0 {
		syscall.CloseHandle(h)
		return nil, e
	}
	t := commTimeouts{0xFFFFFFFF, 0xFFFFFFFF, 100, 0, 1000} // lecturas de 100 ms como maximo
	pSetCommTimeouts.Call(uintptr(h), uintptr(unsafe.Pointer(&t)))
	p := &puerto{h}
	p.lineas(false, false)
	return p, nil
}

func (p *puerto) lineas(dtr, rts bool) error {
	f := func(c uintptr) { pEscapeCommFunction.Call(uintptr(p.h), c) }
	if dtr {
		f(5) // SETDTR
	} else {
		f(6) // CLRDTR
	}
	if rts {
		f(3) // SETRTS
	} else {
		f(4) // CLRRTS
	}
	return nil
}

func (p *puerto) leer(b []byte) (int, error) {
	var n uint32
	err := syscall.ReadFile(p.h, b, &n, nil)
	return int(n), err
}

func (p *puerto) escribir(b []byte) (int, error) {
	var n uint32
	err := syscall.WriteFile(p.h, b, &n, nil)
	return int(n), err
}

func (p *puerto) cerrar() { syscall.CloseHandle(p.h) }
