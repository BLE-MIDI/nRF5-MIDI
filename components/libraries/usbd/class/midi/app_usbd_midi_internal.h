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
#ifndef APP_USBD_MIDI_INTERNAL_H__
#define APP_USBD_MIDI_INTERNAL_H__

#include "app_usbd_audio_types.h"
#include "app_usbd_audio_internal.h"
#include "nrf_ringbuf.h"
#include "app_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup app_usbd_midi_internal USB midi internals
 * @brief @tagAPI52840 USB midi class internals.
 * @ingroup app_usbd_midi
 * @{
 */

/**
 * @brief Forward declaration of type defined by @ref APP_USBD_CLASS_TYPEDEF in midi class.
 *
 */
APP_USBD_CLASS_FORWARD(app_usbd_midi);



/*lint -save -e165*/
/**
 * @brief Forward declaration of @ref app_usbd_midi_user_event_e
 *
 */
enum app_usbd_midi_user_event_e;

/*lint -restore*/

/**
 * @brief User event handler.
 *
 * @param[in] p_inst    Class instance.
 * @param[in] event     User event.
 *
 */
typedef void (*app_usbd_midi_user_ev_handler_t)(app_usbd_class_inst_t const *  p_inst,
                                                enum app_usbd_midi_user_event_e event);


enum app_usbd_midi_rx_event_e;

typedef struct {
    uint8_t * p_data;
    size_t len;
} app_usbd_midi_msg_t;

typedef struct {
    uint8_t data[64];
    size_t  len;
} app_usbd_midi_rx_buf_t;

typedef void (*app_usbd_midi_rx_handler_t)(app_usbd_class_inst_t const * p_inst,
                                        enum app_usbd_midi_rx_event_e event,
                                        uint8_t cable,
                                        app_usbd_midi_msg_t *rx);

/**
 * @brief Midi subclass descriptor.
 */

typedef struct {
   size_t                size;
   uint8_t               type;
   uint8_t const * const p_data;
} app_usbd_midi_subclass_desc_t;

typedef struct {
    uint8_t * p_data;
    size_t pos;
    size_t left; 
} app_usbd_midi_sysex_buf_t;

/**
 * @brief Midi class part of class instance data.
 */
typedef struct {

    app_usbd_midi_subclass_desc_t const * const p_midi_dsc; //!< Audio class Feature Unit descriptor

    uint16_t ep_size;                                       //!< Endpoint size

    app_usbd_audio_subclass_t       type_streaming;         //!< Streaming type MIDISTREAMING/AUDIOSTREAMING (@ref app_usbd_midi_subclass_t)
    nrf_ringbuf_t const *           p_in_buf;               //!< Out queue
    nrf_ringbuf_t const *           p_out_buf;              //!< Out queue
    app_usbd_midi_user_ev_handler_t user_ev_handler;        //!< User event handler
    app_usbd_midi_rx_handler_t      user_rx_handler;        //!< User event handler
} app_usbd_midi_inst_t;


/**
 * @brief Midi class context.
 */
typedef struct {
    app_usbd_audio_req_t        request;       //!< Audio class request.
    bool                        sending;       //!< Sending flag
    bool                        streaming;     //!< Streaming flag
    app_usbd_midi_sysex_buf_t   sysex[16];
    app_usbd_midi_rx_buf_t      rx_transfer[2];
    uint8_t                     rx_buf;
} app_usbd_midi_ctx_t;

/**
 * @brief Midi class configuration macro.
 *
 * Used by @ref APP_USBD_MIDI_GLOBAL_DEF
 *
 * @param iface_control     Interface number of midi control.
 * @param iface_stream      Interface number of midi stream.
 */
#define APP_USBD_MIDI_IN_OUT_CONFIG(iface_control, iface_stream)    \
        ((iface_control),                                           \
         (iface_stream, 0, 1))


/**
 * @brief Midi configuration.
 *
 * @param iface_control     Interface number of midi control.
 * @param iface_stream_in   Interface number of midi stream on IN endpoint.
 */
#define APP_USBD_MIDI_CONFIG_IN_OUT(iface_control, iface_stream_in_out)         \
        (                                                                       \
                (iface_control),                                                \
                (iface_stream_in_out, NRF_DRV_USBD_EPIN1, NRF_DRV_USBD_EPOUT1)  \
        )

/**
 * @brief Specific class constant data for midi class.
 *
 * @ref app_usbd_midi_inst_t
 */
#define APP_USBD_MIDI_INSTANCE_SPECIFIC_DEC app_usbd_midi_inst_t inst;

/**
 * @brief Configures midi class instance.
 *
 * @param user_event_handler        User event handler.
 * @param midi_descriptor           Midi class descriptor.
 * @param ep_siz                    Endpoint size.
 * @param type_str                  Streaming type MIDISTREAMING/AUDIOSTREAMING.
 */
 #define APP_USBD_MIDI_INST_CONFIG(user_event_handler,              \
                                    rx_handler,                     \
                                    midi_descriptor,                \
                                    ep_siz,                         \
                                    type_str,                       \
                                    in_buf)                         \
    .inst = {                                                       \
         .user_ev_handler = user_event_handler,                     \
         .user_rx_handler = rx_handler,                             \
         .p_midi_dsc      = midi_descriptor,                        \
         .ep_size         = ep_siz,                                 \
         .type_streaming  = type_str,                               \
         .p_in_buf        = in_buf,                                 \
    }




/**
 * @brief Specific class data for midi class.
 *
 * @ref app_usbd_midi_ctx_t
 */
#define APP_USBD_MIDI_DATA_SPECIFIC_DEC app_usbd_midi_ctx_t ctx;


/**
 * @brief Public midi class interface.
 *
 */
extern const app_usbd_class_methods_t app_usbd_midi_class_methods;

/**
 * @brief Global definition of @ref app_usbd_midi_t class
 *
 */
#define APP_USBD_MIDI_GLOBAL_DEF_INTERNAL(instance_name,            \
                                    interfaces_configs,             \
                                    user_ev_handler,                \
                                    rx_handler,                     \
                                    midi_descriptor,                \
                                    in_buf_size)                    \
    NRF_RINGBUF_DEF(instance_name##_buf_in, in_buf_size);           \
    APP_USBD_CLASS_INST_GLOBAL_DEF(                                 \
        instance_name,                                              \
        app_usbd_midi,                                              \
        &app_usbd_midi_class_methods,                               \
        interfaces_configs,                                         \
        (APP_USBD_MIDI_INST_CONFIG(user_ev_handler,                 \
                                    rx_handler,                     \
                                    midi_descriptor,                \
                                    0,                              \
                                    APP_USBD_AUDIO_SUBCLASS_MIDISTREAMING, \
                                    &instance_name##_buf_in))       \
    )


/** @} */


#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_MIDI_INTERNAL_H__ */
