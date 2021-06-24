/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef APP_USBD_MIDI_H__
#define APP_USBD_MIDI_H__

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_usbd.h"
#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_class_base.h"
#include "app_usbd_descriptor.h"

#include "app_usbd_midi_types.h"
#include "app_usbd_audio_types.h"
#include "app_usbd_midi_desc.h"
#include "app_usbd_midi_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup app_usbd_midi USB MIDI class
 * @ingroup app_usbd
 *
 * @brief @tagAPI52840 Module with types, definitions, and API used by USB Midi class.
 *
 * @details Reference specifications:
 *
 * @{
 */


#ifdef DOXYGEN
/**
 * @brief Midi class instance type
 *
 * @ref APP_USBD_CLASS_TYPEDEF
 */
typedef struct { } app_usbd_midi_t;
#else
/*lint -save -e10 -e26 -e123 -e505 */
APP_USBD_CLASS_TYPEDEF(app_usbd_midi,    \
    APP_USBD_MIDI_IN_OUT_CONFIG(0, 1),          \
    APP_USBD_MIDI_INSTANCE_SPECIFIC_DEC, \
    APP_USBD_MIDI_DATA_SPECIFIC_DEC      \
);
/*lint -restore*/
#endif

/*lint -save -e407 */

/**
 * @brief Events passed to user event handler
 *
 * @note Example prototype of user event handler:
 *
 * @code
   void midi_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                              app_usbd_midi_user_event_t   event);
 * @endcode
 */

/**
 * @brief Events passed to user event handler
 *
 * @note Example prototype of user event handler:
 *
 * @code
   void (*app_usbd_midi_rx_handler_t)(app_usbd_class_inst_t const * p_inst,
                                        enum app_usbd_midi_rx_event_e event,
                                        uint8_t cable,
                                        app_usbd_midi_msg_t *rx);
 * @endcode
 */

typedef enum app_usbd_midi_user_event_e {
    APP_USBD_MIDI_USER_EVT_CLASS_REQ,
    APP_USBD_MIDI_USER_EVT_TX_DONE,     /**< User event TX_DONE.    */

    APP_USBD_MIDI_USER_EVT_PORT_OPEN,   /**< User event PORT_OPEN.  */
    APP_USBD_MIDI_USER_EVT_PORT_CLOSE,  /**< User event PORT_CLOSE. */
} app_usbd_midi_user_event_t;


typedef enum app_usbd_midi_rx_event_e {
    APP_USBD_MIDI_SYSEX_BUF_REQ,
    APP_USBD_MIDI_SYSEX_RX_DONE,
    APP_USBD_MIDI_RX_DONE,     
} app_usbd_midi_rx_event_t;

/*lint -restore*/

/**
 * @brief Global definition of app_usbd_audio_t class instance.
 *
 * @param instance_name             Name of global instance.
 * @param interfaces_configs        Interfaces configurations.
 * @param user_ev_handler           User event handler.
 * @param midi_descriptor           Midi class Format descriptor.
 *
 * @note This macro is just simplified version of @ref APP_USBD_MIDI_GLOBAL_DEF_INTERNAL
 *
 */
#define APP_USBD_MIDI_GLOBAL_DEF(instance_name,             \
                                  interfaces_configs,       \
                                  user_ev_handler,          \
                                  rx_handler,               \
                                  midi_descriptor,          \
                                  in_buf_size)              \
    APP_USBD_MIDI_GLOBAL_DEF_INTERNAL(instance_name,        \
                                       interfaces_configs,  \
                                       user_ev_handler,     \
                                       rx_handler,          \
                                       midi_descriptor,     \
                                       in_buf_size)         \

/**
 * @brief Initializer of Midi descriptor.
 *
 * @param name  Descriptor name.
 * @param ...   Descriptor data.
*/

#define APP_USBD_MIDI_DESCRIPTOR(name, ...)                     \
    static uint8_t const CONCAT_2(name,  _data)[] =             \
    {                                                           \
        __VA_ARGS__                                             \
    };                                                          \
    static const app_usbd_midi_subclass_desc_t name =           \
    {   sizeof(CONCAT_2(name, _data)),                          \
        APP_USBD_AUDIO_AS_IFACE_SUBTYPE_UNDEFINED,    /* !!! */ \
        CONCAT_2(name,_data)                                    \
    }


/**
 * @@brief Helper function to get class instance from Midi class.
 *
 * @param[in] p_midi Midi class instance (declared by @ref APP_USBD_MIDI_GLOBAL_DEF).
 * @return Base class instance.
 */
static inline app_usbd_class_inst_t const *
app_usbd_midi_class_inst_get(app_usbd_midi_t const * p_midi)
{
    return &p_midi->base;
}

/**
 * @brief Helper function to get midi from base class instance.
 *
 * @param[in] p_inst Base class instance.
 * @return Midi class handle.
 */
static inline app_usbd_midi_t const *
app_usbd_audio_class_get(app_usbd_class_inst_t const * p_inst)
{
    return (app_usbd_midi_t const *)p_inst;
}

/**
 * @brief Write midi data to TX buffer and start sending.
 * 
 * Data has to be a single, complete midi message in order for transfer to be correctly formatted.
 *
 */
ret_code_t app_usbd_midi_write(app_usbd_midi_t const *  p_midi,
                               uint8_t                  cable, 
                               uint8_t *                p_buf,
                               size_t                   len);


/**
 * @brief Write midi sysex data to TX buffer and start sending.
 * 
 * The complete sysex message may be sent using multiple calls to this function.
 * If multiple calls are used, len has to be a multiple of three until the last 
 * fragment of the sysex message, wich may be any length.
 * This makes it possible to store a single sysex message in multiple buffers, 
 * thus enabling support for infinite length sysex messages.
 *
 */
ret_code_t app_usbd_midi_sysex_write(app_usbd_midi_t const *  p_midi,
                               uint8_t                  cable, 
                               uint8_t *                p_buf,
                               size_t                   len);

/**
 * @brief Write raw usb midi data to TX buffer and start sending.
 * 
 * Data passed to this function has to be formated into USB-midi event packets. 
 *
 */
ret_code_t app_usbd_midi_send_raw(app_usbd_midi_t const * p_midi,
                                  const void *        p_buf,
                                  size_t              len);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_MIDI_H__ */
