/**
 * Copyright (c) 2016 - 2020, Nordic Semiconductor ASA
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
#ifndef APP_USBD_MiDI_DESC_H__
#define APP_USBD_MIDI_DESC_H__

#include "app_usbd_descriptor.h"
#include "midi_usbd_descriptors.h"
#include "app_usbd_audio_desc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup app_usbd_midi_dsc USB midi descriptors
 * @brief @tagAPI52840 Descriptors used in the USB midi class.
 * @ingroup app_usbd_midi
 * @{
 */

#define APP_USBD_AUDIO_MIDI_CS_MIDI_STREAMING_INTERFACE_DSC   \
            USBD_CLASS_SPECIFIC_MIDI_STREAMING_INTERFACE_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_EMBEDDED_IN_JACK_DSC \
            USBD_MIDI_EMBEDDED_IN_JACK_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_EXTERNAL_IN_JACK_DSC \
            USBD_MIDI_EXTERNAL_IN_JACK_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_EMBEDDED_OUT_JACK_DSC \
            USBD_MIDI_EMBEDDED_OUT_JACK_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_EXTERNAL_OUT_JACK_DSC \
            USBD_MIDI_EXTERNAL_OUT_JACK_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_STANDARD_BULK_OUT_ENDPOINT_DSC \
            USBD_MIDI_STANDARD_BULK_OUT_ENDPOINT_DESCRIPTOR

#define APP_USBD_AUDIO_MIDI_BULK_OUT_ENDPOINT_DSC \
            USBD_MIDI_CLASS_SPECIFIC_BULK_OUT_ENDPOINT_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_STANDARD_BULK_IN_ENDPOINT_DSC \
            USBD_MIDI_STANDARD_BULK_IN_ENDPOINT_DESCRIPTOR


#define APP_USBD_AUDIO_MIDI_BULK_IN_ENDPOINT_DSC \
            USBD_MIDI_CLASS_SPECIFIC_BULK_IN_ENDPOINT_DESCRIPTOR

            

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_MIDI_DESC_H__ */
