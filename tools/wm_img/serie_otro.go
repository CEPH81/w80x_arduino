//go:build !linux && !windows

package main

import "errors"

type puerto struct{}

func abrirPuerto(string) (*puerto, error)        { return nil, errors.New("sistema no soportado") }
func (p *puerto) lineas(bool, bool) error        { return nil }
func (p *puerto) leer([]byte) (int, error)       { return 0, nil }
func (p *puerto) escribir(b []byte) (int, error) { return len(b), nil }
func (p *puerto) cerrar()                        {}
