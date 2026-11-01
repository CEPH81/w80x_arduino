//go:build linux

// serie_linux.go - puerto serie minimo (115200 8N1, sin control de flujo) para Linux.
package main

import (
	"syscall"
	"unsafe"
)

type puerto struct{ fd int }

func ioctl(fd int, req uintptr, arg unsafe.Pointer) error {
	_, _, e := syscall.Syscall(syscall.SYS_IOCTL, uintptr(fd), req, uintptr(arg))
	if e != 0 {
		return e
	}
	return nil
}

func abrirPuerto(nombre string) (*puerto, error) {
	fd, err := syscall.Open(nombre, syscall.O_RDWR|syscall.O_NOCTTY|syscall.O_NONBLOCK, 0)
	if err != nil {
		return nil, err
	}
	p := &puerto{fd}
	// lineas DTR y RTS inactivas cuanto antes (abrir el puerto las activa)
	p.lineas(false, false)
	var t syscall.Termios
	if err := ioctl(fd, syscall.TCGETS, unsafe.Pointer(&t)); err != nil {
		syscall.Close(fd)
		return nil, err
	}
	t.Iflag = 0
	t.Oflag = 0
	t.Lflag = 0
	t.Cflag = syscall.CS8 | syscall.CREAD | syscall.CLOCAL | syscall.B115200
	t.Ispeed = syscall.B115200
	t.Ospeed = syscall.B115200
	t.Cc[syscall.VMIN] = 0
	t.Cc[syscall.VTIME] = 1 // lecturas de 100 ms como maximo
	if err := ioctl(fd, syscall.TCSETS, unsafe.Pointer(&t)); err != nil {
		syscall.Close(fd)
		return nil, err
	}
	syscall.SetNonblock(fd, false)
	return p, nil
}

func (p *puerto) lineas(dtr, rts bool) error {
	var bits int32
	if err := ioctl(p.fd, syscall.TIOCMGET, unsafe.Pointer(&bits)); err != nil {
		return err
	}
	bits &^= syscall.TIOCM_DTR | syscall.TIOCM_RTS
	if dtr {
		bits |= syscall.TIOCM_DTR
	}
	if rts {
		bits |= syscall.TIOCM_RTS
	}
	return ioctl(p.fd, syscall.TIOCMSET, unsafe.Pointer(&bits))
}

func (p *puerto) leer(b []byte) (int, error) {
	n, err := syscall.Read(p.fd, b)
	if n < 0 {
		n = 0
	}
	return n, err
}

func (p *puerto) escribir(b []byte) (int, error) { return syscall.Write(p.fd, b) }
func (p *puerto) cerrar()                        { syscall.Close(p.fd) }
