/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Armink (armink.ztl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <rtthread.h>
#include <drivers/pin.h>

#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "modmachine.h"

#if MICROPY_PY_PIN

#define GPIO_MODE_IN                           ((uint32_t)0x00000000)   /*!< Input Floating Mode                   */
#define GPIO_MODE_OUT_PP                       ((uint32_t)0x00000001)   /*!< Output Push Pull Mode                 */
#define GPIO_MODE_OUT_OD                       ((uint32_t)0x00000011)   /*!< Output Open Drain Mode                */
#define GPIO_MODE_AF_PP                        ((uint32_t)0x00000002)   /*!< Alternate Function Push Pull Mode     */
#define GPIO_MODE_AF_OD                        ((uint32_t)0x00000012)   /*!< Alternate Function Open Drain Mode    */
#define GPIO_MODE_ANALOG                       ((uint32_t)0x00000003)   /*!< Analog Mode  */
#define GPIO_NOPULL                            ((uint32_t)0x00000000)   /*!< No Pull-up or Pull-down activation  */
#define GPIO_PULLUP                            ((uint32_t)0x00000001)   /*!< Pull-up activation                  */
#define GPIO_PULLDOWN                          ((uint32_t)0x00000002)   /*!< Pull-down activation                */
#define GPIO_MODE_IT_RISING                    ((uint32_t)0x10110000)   /*!< External Interrupt Mode with Rising edge trigger detection          */
#define GPIO_MODE_IT_FALLING                   ((uint32_t)0x10210000)   /*!< External Interrupt Mode with Falling edge trigger detection         */
#define GPIO_MODE_IT_RISING_FALLING            ((uint32_t)0x10310000)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */

const mp_obj_base_t machine_pin_obj_template = {&machine_pin_type};

void mp_pin_od_write(void *machine_pin, int stat) {
    if (stat == PIN_LOW) {
        rt_pin_mode(((machine_pin_obj_t *)machine_pin)->pin, PIN_MODE_OUTPUT);
        rt_pin_write(((machine_pin_obj_t *)machine_pin)->pin, stat);
    } else {
        rt_pin_mode(((machine_pin_obj_t *)machine_pin)->pin, PIN_MODE_INPUT_PULLUP);
    }
}

void mp_hal_pin_open_set(void *machine_pin, int mode) {
    rt_pin_mode(((machine_pin_obj_t *)machine_pin)->pin, mode);
}

char* mp_hal_pin_get_name(void *machine_pin) {
    return ((machine_pin_obj_t *)machine_pin)->name;
}

STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "<Pin %d>", self->pin);
}

// constructor(drv_name, pin, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted port
    if (!MP_OBJ_IS_TYPE(args[0], &mp_type_tuple)) {
        mp_raise_ValueError("Pin id must be tuple of (\"GPIO_x\", pin#)");
    }
    mp_obj_t *items;
    mp_obj_get_array_fixed_n(args[0], 2, &items);
    const char *pin_name = mp_obj_str_get_str(items[0]);
    int wanted_pin = mp_obj_get_int(items[1]);

    machine_pin_obj_t *pin = m_new_obj(machine_pin_obj_t);
    if (!pin) {
        mp_raise_OSError(ENOMEM);
    }

    strncpy(pin->name, pin_name, sizeof(pin->name));
    pin->base = machine_pin_obj_template;
    pin->pin = wanted_pin;

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)pin;
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get io mode
    uint mode = args[ARG_mode].u_int;

    // get pull mode
    uint pull = GPIO_NOPULL;

    if (args[ARG_pull].u_obj != mp_const_none) {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
    }

    switch(mode) {
    case GPIO_MODE_IN: {
        if (pull == GPIO_PULLUP) {
            mode = PIN_MODE_INPUT_PULLUP;
        } else if (pull == GPIO_PULLDOWN) {
            mode = PIN_MODE_INPUT_PULLDOWN;
        } else {
            mode = PIN_MODE_INPUT;
        }
        break;
    }
    case GPIO_MODE_OUT_PP : {
        mode = PIN_MODE_OUTPUT;
        break;
    }
    case GPIO_MODE_OUT_OD : {
        mode = PIN_MODE_OUTPUT_OD;
        break;
    }
    case GPIO_MODE_AF_PP :
    case GPIO_MODE_AF_OD :
    case GPIO_MODE_ANALOG :
        //TODO
        mp_raise_NotImplementedError("not implemented pin mode");
    }

    rt_pin_mode(self->pin, mode);

    // get initial value
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        rt_pin_write(self->pin, mp_obj_is_true(args[ARG_value].u_obj));
    }

    return mp_const_none;
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0) {
        return mp_obj_new_bool(rt_pin_read(self->pin));
    } else {
        rt_pin_write(self->pin, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// pin.name()
STATIC mp_obj_t machine_pin_name(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = (machine_pin_obj_t *)args[0];
    return mp_obj_new_str(self->name, strlen(self->name));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_name_obj, 1, 2, machine_pin_name);

// pin.pin()
STATIC mp_obj_t machine_pin_pin(size_t n_args, const mp_obj_t *args) {
    return MP_OBJ_NEW_SMALL_INT(((machine_pin_obj_t *)args[0])->pin);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_pin_obj, 1, 2, machine_pin_pin);

STATIC mp_uint_t machine_pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;

    switch (request) {
        case MP_PIN_READ: {
            uint32_t pin_val = rt_pin_read(self->pin);
            return pin_val;
        }
        case MP_PIN_WRITE: {
            rt_pin_write(self->pin, arg);
            return 0;
        }
    }
    return -1;
}

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init),    MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value),   MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_name),    MP_ROM_PTR(&machine_pin_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_pin),     MP_ROM_PTR(&machine_pin_pin_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_ALT_OD),    MP_ROM_INT(GPIO_MODE_AF_OD) },
    { MP_ROM_QSTR(MP_QSTR_ALT_PP),    MP_ROM_INT(GPIO_MODE_AF_PP) },
    { MP_ROM_QSTR(MP_QSTR_ANALOG),    MP_ROM_INT(GPIO_MODE_ANALOG) },
    { MP_ROM_QSTR(MP_QSTR_IN),        MP_ROM_INT(GPIO_MODE_IN) },
    { MP_ROM_QSTR(MP_QSTR_OUT_PP),    MP_ROM_INT(GPIO_MODE_OUT_PP) },
    { MP_ROM_QSTR(MP_QSTR_OUT_OD),    MP_ROM_INT(GPIO_MODE_OUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULLDOWN) },
    { MP_ROM_QSTR(MP_QSTR_PULL_NONE), MP_ROM_INT(GPIO_NOPULL) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP),   MP_ROM_INT(GPIO_PULLUP) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(GPIO_MODE_IT_RISING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(GPIO_MODE_IT_FALLING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING_FALLING), MP_ROM_INT(GPIO_MODE_IT_RISING_FALLING) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

STATIC const mp_pin_p_t machine_pin_pin_p = {
    .ioctl = machine_pin_ioctl,
};

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
    .call = machine_pin_call,
    .protocol = &machine_pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};

#endif
