package main

var defaultInstInfo = [256][]byte{
	[]byte{},
	[]byte{0, 1, 5, 4, 4},
	[]byte{4},
	nil,
	[]byte{10, 8},
	[]byte{},
	[]byte{4},
	[]byte{10, 8},
	[]byte{0},
	[]byte{0, 1, 5},
	[]byte{1, 5},
	[]byte{1, 0},
	[]byte{1, 0, 7, 1},
	[]byte{1, 1, 5},
	[]byte{1, 1, 0},
	[]byte{0},
	nil,
	[]byte{6, 8, 5},
	[]byte{6, 8, 0, 10, 8},
	[]byte{},
	[]byte{4, 6, 8, 6, 8, 0},
	[]byte{6, 8, 0},
	[]byte{0, 0},
	[]byte{},
	[]byte{0, 6, 8},
	[]byte{},
	[]byte{6, 8},
	[]byte{0},
	[]byte{6, 8, 6, 8, 1, 0},
	[]byte{1},
	[]byte{6, 8, 10, 8, 5, 5, 1, 1, 0, 5},
	[]byte{6, 8, 5},
	[]byte{6, 8, 5, 1},
	[]byte{6, 8, 1, 1, 1},
	[]byte{6, 8, 0},
	nil,
	nil,
	nil,
	nil,
	nil,
	[]byte{6, 8, 10, 8, 5, 5, 1, 1, 0, 1, 1, 0, 5},
	[]byte{6, 8, 5},
	[]byte{6, 8, 5, 1},
	[]byte{6, 8},
	[]byte{6, 8},
	[]byte{6, 8, 0},
	[]byte{},
	[]byte{6, 8, 1, 5},
	nil,
	nil,
	[]byte{10, 8},
	[]byte{6, 8, 10, 8, 0, 0},
	[]byte{6, 8, 10, 8, 0, 0},
	[]byte{6, 8, 10, 8, 0, 0, 0},
	[]byte{6, 8, 5, 5, 5, 5, 5, 5, 5, 0, 0},
	[]byte{6, 8},
	[]byte{6, 8, 0},
	[]byte{6, 8, 0, 0, 7, 1},
	[]byte{6, 8, 0, 0},
	[]byte{6, 8, 6, 8, 1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5},
	[]byte{6, 8},
	[]byte{1},
	[]byte{},
	[]byte{7, 6},
	[]byte{6, 8, 10, 8, 0},
	[]byte{6, 8, 0},
	[]byte{6, 8, 1},
	[]byte{6, 8},
	[]byte{6, 8, 6, 8, 0},
	[]byte{6, 8, 1, 5, 5, 5, 5},
	[]byte{6, 8, 1, 0, 5, 5, 5, 5},
	[]byte{6, 8, 6, 8, 1, 0, 0, 5, 5, 5, 5, 5, 1, 5},
	[]byte{6, 8, 6, 8, 1, 0, 0, 10, 8},
	[]byte{6, 8, 6, 8, 10, 8},
	[]byte{6, 8, 6, 8},
	[]byte{6, 8, 1, 1, 5, 5, 5, 5},
	[]byte{6, 8, 1, 1, 0, 5, 5, 5, 5},
	[]byte{6, 8, 6, 8, 1, 1, 0, 0, 5, 5, 5, 5, 5, 1, 5},
	[]byte{6, 8, 6, 8, 1, 1, 0, 0, 10, 8},
	[]byte{6, 8, 6, 8, 1, 10, 8},
	[]byte{6, 8, 6, 8, 1},
	[]byte{6, 8, 6, 8, 1, 5, 0},
	[]byte{6, 8, 6, 8, 5, 1, 5, 0, 10, 8},
	[]byte{6, 8, 6, 8},
	[]byte{6, 8, 6, 8, 10, 8},
	[]byte{6, 8, 6, 8},
	[]byte{6, 8, 0, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 0, 1, 6, 8, 1, 6, 8, 10, 8, 5},
	[]byte{6, 8, 1},
	[]byte{6, 8, 6, 8},
	[]byte{6, 8, 6, 8, 1},
	[]byte{6, 8, 7, 1},
	[]byte{6, 8, 1, 0},
	[]byte{6, 8},
	[]byte{6, 8, 6, 8, 0},
	[]byte{6, 8, 5, 5},
	[]byte{10, 8},
	[]byte{1, 1, 1, 1},
	[]byte{0, 5, 5, 5, 5},
	[]byte{6, 8},
	[]byte{6, 8, 0},
	[]byte{0},
	[]byte{1, 0, 5, 5, 0, 10, 8},
	[]byte{10, 8},
	[]byte{0, 0, 1, 5, 5, 5, 5, 5, 0},
	[]byte{0},
	[]byte{6, 8, 0, 0, 5, 5, 5, 5, 5, 1, 5},
	[]byte{6, 8, 1, 0, 0, 10, 8},
	[]byte{6, 8, 6, 8},
	nil,
	nil,
	[]byte{9, 8, 9, 8},
	[]byte{9, 8},
	[]byte{9, 8, 1},
	[]byte{},
	[]byte{9, 8, 1, 1, 9, 8},
	[]byte{9, 8, 9, 8, 1},
	[]byte{9, 8, 9, 8},
	[]byte{9, 8, 6, 8},
	nil,
	nil,
	[]byte{6, 8, 10, 8, 0, 0, 0},
	[]byte{6, 8, 6, 8, 5},
	[]byte{6, 8, 10, 8, 5, 0, 0, 10, 8},
	[]byte{6, 8, 10, 8},
	[]byte{6, 8, 6, 8, 5},
	[]byte{6, 8, 5},
	[]byte{6, 8},
	[]byte{6, 8, 5, 5, 5, 5, 5},
	[]byte{6, 8},
	[]byte{6, 8, 0, 10, 8, 5, 5, 0},
	[]byte{6, 8, 10, 8, 5},
	[]byte{6, 8, 6, 8, 5, 5},
	[]byte{6, 8, 6, 8, 6, 8, 5},
	[]byte{6, 8, 6, 8, 0, 5},
	[]byte{6, 8, 5, 5, 5},
	nil,
	nil,
	nil,
	nil,
	nil,
	[]byte{6, 8, 10, 8, 6, 8, 0, 0},
	[]byte{4, 6, 8, 6, 8, 0, 0, 10, 8},
	[]byte{6, 8, 6, 8, 0, 0, 10, 8},
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	[]byte{},
	[]byte{6, 8, 6, 8, 1, 1, 1},
	[]byte{6, 8, 6, 8},
	[]byte{6, 8, 0, 0},
	[]byte{},
	[]byte{6, 8, 6, 8, 6, 8, 6, 8, 6, 8, 5, 0},
	[]byte{0},
	[]byte{6, 8, 6, 8, 5},
	[]byte{6, 8, 1},
	[]byte{6, 8, 1},
	[]byte{6, 8},
	[]byte{6, 8},
	[]byte{10, 8, 1, 1},
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	[]byte{0},
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	[]byte{},
	[]byte{0, 10, 8},
	[]byte{},
	[]byte{0},
	[]byte{1},
	[]byte{},
	[]byte{6, 8},
	nil,
}
