// carga.go - "wm_img serial": carga por puerto serie con wm_tool y reinicio fiable al terminar.
//
// wm_tool (-rs rts) reinicia el chip despues de grabar con la misma secuencia de DTR/RTS
// que usa para entrar en modo de carga, asi que en placas con DTR conectado a BOOT
// (HLK-W801-KIT, HLK-W806-KIT...) el chip vuelve a quedarse en la ROM de carga y hay que
// pulsar RST. Aqui, al terminar wm_tool, se abre el puerto con DTR y RTS inactivos y, si
// la ROM sigue esperando (envia 'C'), se le manda la orden de reinicio, la misma que usa
// el programa de carga oficial del WM IoT SDK (tools/wm/flash.py).
//
// Uso: wm_img serial -tool <wm_tool> -port <puerto> -- <argumentos de wm_tool>
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"time"
)

// orden "reboot" de la ROM de carga (WM IoT SDK, tools/wm/flash.py)
var ordenReinicio = []byte{0x21, 0x06, 0x00, 0xc7, 0x7c, 0x3f, 0x00, 0x00, 0x00}

func serialMain(args []string) {
	fs := flag.NewFlagSet("serial", flag.ExitOnError)
	tool := fs.String("tool", "", "ruta de wm_tool")
	port := fs.String("port", "", "puerto serie")
	reset := fs.Bool("reset", true, "reiniciar el chip al terminar")
	fs.Parse(args)
	if *tool == "" || *port == "" {
		fail("uso: wm_img serial -tool <wm_tool> -port <puerto> -- <argumentos de wm_tool>")
	}
	cmd := exec.Command(*tool, fs.Args()...)
	cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
	if err := cmd.Run(); err != nil {
		if ee, ok := err.(*exec.ExitError); ok {
			os.Exit(ee.ExitCode())
		}
		fail("no se puede ejecutar %s: %v", *tool, err)
	}
	if *reset {
		reiniciar(*port)
	}
}

// reiniciar abre el puerto (DTR y RTS inactivos) y saca al chip de la ROM de carga.
func reiniciar(nombre string) {
	var p *puerto
	var err error
	for i := 0; i < 30; i++ { // el sistema tarda un poco en liberar el puerto
		if p, err = abrirPuerto(nombre); err == nil {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if err != nil {
		fmt.Printf("No se pudo abrir %s para reiniciar el chip (%v): pulse RST.\n", nombre, err)
		return
	}
	defer p.cerrar()

	c, otros := escuchar(p, 700*time.Millisecond)
	if c < 3 && otros > 0 {
		fmt.Println("Programa en marcha.")
		return
	}
	p.escribir(ordenReinicio)
	time.Sleep(50 * time.Millisecond)
	c, _ = escuchar(p, 600*time.Millisecond)
	if c >= 3 {
		fmt.Println("El chip sigue en modo de carga: pulse RST.")
		return
	}
	fmt.Println("Chip reiniciado: programa en marcha.")
}

// escuchar cuenta las 'C' seguidas (la ROM de carga las envia sin parar) y los demas bytes.
func escuchar(p *puerto, dur time.Duration) (maxC, otros int) {
	buf := make([]byte, 256)
	seguidas := 0
	fin := time.Now().Add(dur)
	for time.Now().Before(fin) {
		n, _ := p.leer(buf)
		if n == 0 {
			time.Sleep(10 * time.Millisecond)
			continue
		}
		for _, b := range buf[:n] {
			if b == 'C' {
				seguidas++
				if seguidas > maxC {
					maxC = seguidas
				}
			} else {
				seguidas = 0
				otros++
			}
		}
	}
	return
}
