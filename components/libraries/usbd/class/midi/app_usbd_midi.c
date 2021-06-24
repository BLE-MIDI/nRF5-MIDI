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
#include "sdk_common.h"
#define APP_USBD_MIDI_ENABLED 1
#if NRF_MODULE_ENABLED(APP_USBD_MIDI)

#include "app_usbd_midi.h"
#include "app_util_platform.h"

/**
 * @defgroup app_usbd_midi_internals USBD Midi internals
 * @{
 * @ingroup app_usbd_midi
 * @internal
 */


#define USBD_MIDI_EVENT_SIZE 4

#define APP_USBD_AUDIO_CONTROL_IFACE_IDX    0 /**< Audio class control interface index */
#define APP_USBD_MIDI_STREAMING_IFACE_IDX   1 /**< Midi class midi streaming interface index */
#define APP_USBD_MIDI_STREAMING_EP_OUT_IDX  1 /**< Midi streaming bulk endpoint out index */
#define APP_USBD_MIDI_STREAMING_EP_IN_IDX   2 /**< Midi streaming bulk endpoint in index */

/**
 * @brief Midi rx buffer
 */
static uint8_t m_rx_buffer[USBD_MIDI_EVENT_SIZE];
NRF_DRV_USBD_TRANSFER_OUT(rx_transfer, m_rx_buffer, USBD_MIDI_EVENT_SIZE);

/**
 * @brief Auxiliary function to access midi class instance data.
 *
 * @param[in] p_inst Class instance data.
 * @return Midi class instance data @ref app_usbd_midi_t
 */
static inline app_usbd_midi_t const * midi_get(app_usbd_class_inst_t const * p_inst)
{
    ASSERT(p_inst != NULL);
    return (app_usbd_midi_t const *)p_inst;
}


/**
 * @brief Auxiliary function to access midi class context data.
 *
 * @param[in] p_midi    Midi class instance data.
 * @return Midi class context data @ref app_usbd_midi_ctx_t
 */
static inline app_usbd_midi_ctx_t * midi_ctx_get(app_usbd_midi_t const * p_midi)
{
    ASSERT(p_midi != NULL);
    ASSERT(p_midi->specific.p_data != NULL);
    return &p_midi->specific.p_data->ctx;
}


/**
 * @brief User event handler.
 *
 * @param[in] p_inst        Class instance.
 * @param[in] event user    Event type @ref app_usbd_midi_user_event_t
 */
static inline void user_event_handler(
    app_usbd_class_inst_t const * p_inst,
    app_usbd_midi_user_event_t   event)
{
    app_usbd_midi_t const * p_midi = midi_get(p_inst);

    if (p_midi->specific.inst.user_ev_handler != NULL)
    {
        p_midi->specific.inst.user_ev_handler(p_inst, event);
    }
}

/**
 * @brief User event handler.
 *
 * @param[in] p_inst        Class instance.
 * @param[in] event user    Event type @ref app_usbd_midi_user_event_t
 */
static inline void user_rx_handler(app_usbd_class_inst_t const * p_inst,
                                        enum app_usbd_midi_rx_event_e event,
                                        uint8_t cable,
                                        app_usbd_midi_msg_t *rx)
{
    app_usbd_midi_t const * p_midi = midi_get(p_inst);

    if (p_midi->specific.inst.user_rx_handler != NULL)
    {
        p_midi->specific.inst.user_rx_handler(p_inst, event, cable, rx);
    }
}

static bool midi_consumer(nrf_drv_usbd_ep_transfer_t * p_next,
                             void *                       p_context,
                             size_t                       ep_size,
                             size_t                       data_size)
{
    app_usbd_midi_ctx_t  * p_midi_ctx = (app_usbd_midi_ctx_t *) p_context;
    
    p_next->size = data_size;
    static uint8_t buffer = 0;

    p_midi_ctx->rx_buf = buffer;
    p_next->p_data.rx = p_midi_ctx->rx_transfer[buffer].data;
    p_midi_ctx->rx_transfer[buffer].len = data_size;

    if (buffer == 0) {
        buffer = 1;
    } else {
        buffer = 0;
    }

    return false;
}

/**
 * @brief Select interface.
 *
 * @param[in,out] p_inst    Instance of the class.
 * @param[in]     iface_idx Index of the interface inside class structure.
 * @param[in]     alternate Alternate setting that should be selected.
 */
static ret_code_t iface_select(
    app_usbd_class_inst_t const * const p_inst,
    uint8_t                             iface_idx,
    uint8_t                             alternate)
{
    app_usbd_class_iface_conf_t const * p_iface = app_usbd_class_iface_get(p_inst, iface_idx);
    /* Simple check if this is data interface */
    uint8_t const ep_count = app_usbd_class_iface_ep_count_get(p_iface);

    if (ep_count > 0)
    {
        if (alternate > 1)
        {
            return NRF_ERROR_INVALID_PARAM;
        }
        app_usbd_midi_t const * p_midi     = midi_get(p_inst);
        app_usbd_midi_ctx_t   * p_midi_ctx = midi_ctx_get(p_midi);
        p_midi_ctx->streaming = (alternate == 0);

        uint8_t i;

        for (i = 0; i < ep_count; ++i)
        {
            nrf_drv_usbd_ep_t ep_addr =
                app_usbd_class_ep_address_get(app_usbd_class_iface_ep_get(p_iface, i));
            if (alternate == 0)
            {
                app_usbd_ep_enable(ep_addr);
                if (ep_addr == NRF_DRV_USBD_EPOUT1)
                {
                    nrf_drv_usbd_handler_desc_t handler_desc = {
                        .handler.consumer = midi_consumer,
                        .p_context        = p_midi_ctx
                    };

                    app_usbd_ep_handled_transfer(NRF_DRV_USBD_EPOUT1, &handler_desc);


                    user_event_handler(p_inst,
                        APP_USBD_MIDI_USER_EVT_PORT_OPEN);
                }
                if (ep_addr == NRF_DRV_USBD_EPIN1)
                {
                    nrf_ringbuf_init(p_midi->specific.inst.p_in_buf);
                }
            }
            else
            {
                app_usbd_ep_disable(ep_addr);
                user_event_handler(p_inst,
                    APP_USBD_MIDI_USER_EVT_PORT_CLOSE);
            }
        }
        return NRF_SUCCESS;
    }
    return NRF_ERROR_NOT_SUPPORTED;
}


static void iface_deselect(
    app_usbd_class_inst_t const * const p_inst,
    uint8_t                             iface_idx)
{
    app_usbd_class_iface_conf_t const * p_iface = app_usbd_class_iface_get(p_inst, iface_idx);

    /* Simple check if this is data interface */
    if (p_iface->ep_cnt > 0)
    {
        app_usbd_midi_t const * p_midi     = midi_get(p_inst);
        app_usbd_midi_ctx_t   * p_midi_ctx = midi_ctx_get(p_midi);
        p_midi_ctx->streaming = false;
        user_event_handler(p_inst,
                    APP_USBD_MIDI_USER_EVT_PORT_CLOSE);
    }
    /* Note that all the interface endpoints would be disabled automatically after this function */
}

static uint8_t iface_selection_get(
    app_usbd_class_inst_t const * const p_inst,
    uint8_t                             iface_idx)
{
    app_usbd_class_iface_conf_t const * p_iface = app_usbd_class_iface_get(p_inst, iface_idx);
    /* Simple check if this is data interface */
    uint8_t const ep_count = app_usbd_class_iface_ep_count_get(p_iface);

    if (ep_count > 0)
    {
        app_usbd_midi_t const * p_midi     = midi_get(p_inst);
        app_usbd_midi_ctx_t   * p_midi_ctx = midi_ctx_get(p_midi);
        return (p_midi_ctx->streaming) ? 1 : 0;
    }
    return 0;
}

/**
 * @brief Internal SETUP standard IN request handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 * @retval NRF_SUCCESS              Request handled correctly.
 * @retval NRF_ERROR_NOT_SUPPORTED  Request is not supported.
 */
static ret_code_t setup_req_std_in(app_usbd_class_inst_t const * p_inst,
                                   app_usbd_setup_evt_t const  * p_setup_ev)
{

    /* Only Get Descriptor standard IN request is supported by Audio class */
    if ((app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType) == APP_USBD_SETUP_REQREC_INTERFACE)
        &&
        (p_setup_ev->setup.bRequest == APP_USBD_SETUP_STDREQ_GET_DESCRIPTOR))
    {
        size_t dsc_len = 0;
        size_t max_size;

        uint8_t * p_trans_buff = app_usbd_core_setup_transfer_buff_get(&max_size);

        /* Try to find descriptor in class internals*/
        ret_code_t ret = app_usbd_class_descriptor_find(
            p_inst,
            p_setup_ev->setup.wValue.hb,
            p_setup_ev->setup.wValue.lb,
            p_trans_buff,
            &dsc_len);

        if (ret != NRF_ERROR_NOT_FOUND)
        {
            ASSERT(dsc_len < NRF_DRV_USBD_EPSIZE);
            return app_usbd_core_setup_rsp(&(p_setup_ev->setup), p_trans_buff, dsc_len);
        }
    }

    return NRF_ERROR_NOT_SUPPORTED;
}

/**
 * @brief Internal SETUP class IN request handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 * @retval NRF_SUCCESS              Request handled correctly.
 * @retval NRF_ERROR_NOT_SUPPORTED  Request is not supported.
 */
static ret_code_t midi_setup_req_class_in(
    app_usbd_class_inst_t const * p_inst,
    app_usbd_setup_evt_t const  * p_setup_ev)
{
    switch (p_setup_ev->setup.bRequest)
    {

        case APP_USBD_AUDIO_REQ_GET_CUR:
        case APP_USBD_AUDIO_REQ_GET_MIN:
        case APP_USBD_AUDIO_REQ_GET_MAX:
        case APP_USBD_AUDIO_REQ_SET_RES:
        case APP_USBD_AUDIO_REQ_GET_MEM:
        {
            app_usbd_midi_t const * p_midi     = midi_get(p_inst);
            app_usbd_midi_ctx_t   * p_midi_ctx = midi_ctx_get(p_midi);


            p_midi_ctx->request.req_type  = (app_usbd_audio_req_type_t)p_setup_ev->setup.bRequest;
            p_midi_ctx->request.control   = p_setup_ev->setup.wValue.hb;
            p_midi_ctx->request.channel   = p_setup_ev->setup.wValue.lb;
            p_midi_ctx->request.interface = p_setup_ev->setup.wIndex.hb;
            p_midi_ctx->request.entity    = p_setup_ev->setup.wIndex.lb;

            p_midi_ctx->request.length = p_setup_ev->setup.wLength.w;

            p_midi_ctx->request.req_target = APP_USBD_AUDIO_CLASS_REQ_IN;

            app_usbd_setup_reqrec_t rec = app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType);
            if (rec == APP_USBD_SETUP_REQREC_ENDPOINT)
            {
                p_midi_ctx->request.req_target = APP_USBD_AUDIO_EP_REQ_IN;
            }

            user_event_handler((app_usbd_class_inst_t const *)p_midi,
                               APP_USBD_MIDI_USER_EVT_CLASS_REQ);

            return app_usbd_core_setup_rsp(&p_setup_ev->setup,
                                           p_midi_ctx->request.payload,
                                           p_midi_ctx->request.length);
        }

        default:
            break;
    }

    return NRF_ERROR_NOT_SUPPORTED;
}

static ret_code_t midi_req_out_data_cb(nrf_drv_usbd_ep_status_t status, void * p_context)
{
    if (status == NRF_USBD_EP_OK)
    {
        app_usbd_midi_t const * p_midi = p_context;

        user_event_handler((app_usbd_class_inst_t const *)p_midi,
                           APP_USBD_MIDI_USER_EVT_CLASS_REQ);
    }

    return NRF_SUCCESS;
}

static ret_code_t midi_req_out(
    app_usbd_class_inst_t const * p_inst,
    app_usbd_setup_evt_t const  * p_setup_ev)
{
    app_usbd_midi_t const * p_midi     = midi_get(p_inst);
    app_usbd_midi_ctx_t   * p_midi_ctx = midi_ctx_get(p_midi);



    p_midi_ctx->request.req_type  = (app_usbd_audio_req_type_t)p_setup_ev->setup.bRequest;
    p_midi_ctx->request.control   = p_setup_ev->setup.wValue.hb;
    p_midi_ctx->request.channel   = p_setup_ev->setup.wValue.lb;
    p_midi_ctx->request.interface = p_setup_ev->setup.wIndex.hb;
    p_midi_ctx->request.entity    = p_setup_ev->setup.wIndex.lb;

    p_midi_ctx->request.length = p_setup_ev->setup.wLength.w;

    p_midi_ctx->request.req_target = APP_USBD_AUDIO_CLASS_REQ_OUT;
    if (app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType) == APP_USBD_SETUP_REQREC_ENDPOINT)
    {
        p_midi_ctx->request.req_target = APP_USBD_AUDIO_EP_REQ_OUT;
    }

    /*Request setup data*/
    NRF_DRV_USBD_TRANSFER_OUT(transfer, p_midi_ctx->request.payload, p_midi_ctx->request.length);
    ret_code_t ret;
    CRITICAL_REGION_ENTER();
    ret = app_usbd_ep_transfer(NRF_DRV_USBD_EPOUT0, &transfer);
    if (ret == NRF_SUCCESS)
    {
        app_usbd_core_setup_data_handler_desc_t desc = {
            .handler   = midi_req_out_data_cb,
            .p_context = (void *)p_midi
        };

        ret = app_usbd_core_setup_data_handler_set(NRF_DRV_USBD_EPOUT0, &desc);
    }
    CRITICAL_REGION_EXIT();
    return ret;
}

/**
 * @brief Internal SETUP class OUT request handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code
 * @retval NRF_SUCCESS              Request handled correctly.
 * @retval NRF_ERROR_NOT_SUPPORTED  Request is not supported.
 */
static ret_code_t midi_setup_req_class_out(
    app_usbd_class_inst_t const * p_inst,
    app_usbd_setup_evt_t const  * p_setup_ev)
{
    switch (p_setup_ev->setup.bRequest)
    {
        case APP_USBD_AUDIO_REQ_SET_CUR:
        case APP_USBD_AUDIO_REQ_SET_MIN:
        case APP_USBD_AUDIO_REQ_SET_MAX:
        case APP_USBD_AUDIO_REQ_SET_RES:
        case APP_USBD_AUDIO_REQ_SET_MEM:
            return midi_req_out(p_inst, p_setup_ev);

        default:
            break;
    }

    return NRF_ERROR_NOT_SUPPORTED;
}

/**
 * @brief Control endpoint handle.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 * @retval NRF_SUCCESS              Request handled correctly.
 * @retval NRF_ERROR_NOT_SUPPORTED  Request is not supported.
 */
static ret_code_t setup_event_handler(
    app_usbd_class_inst_t const * p_inst,
    app_usbd_setup_evt_t const  * p_setup_ev)
{
    ASSERT(p_inst != NULL);
    ASSERT(p_setup_ev != NULL);
   if (app_usbd_setup_req_dir(p_setup_ev->setup.bmRequestType) == APP_USBD_SETUP_REQDIR_IN)
    {
        switch (app_usbd_setup_req_typ(p_setup_ev->setup.bmRequestType))
        {
            case APP_USBD_SETUP_REQTYPE_STD:
                return setup_req_std_in(p_inst, p_setup_ev);

            case APP_USBD_SETUP_REQTYPE_CLASS:
                return midi_setup_req_class_in(p_inst, p_setup_ev);

            default:
                break;
        }
    }
    else /*APP_USBD_SETUP_REQDIR_OUT*/
    {
        switch (app_usbd_setup_req_typ(p_setup_ev->setup.bmRequestType))
        {
            case APP_USBD_SETUP_REQTYPE_CLASS:
                return midi_setup_req_class_out(p_inst, p_setup_ev);

            default:
                break;
        }
    }

    return NRF_ERROR_NOT_SUPPORTED;
}

/**
 * @brief Auxiliary function to access midi out endpoint address.
 *
 * @param[in] p_inst Class instance data.
 *
 * @return OUT endpoint address.
 */
static inline nrf_drv_usbd_ep_t ep_out_addr_get(app_usbd_class_inst_t const * p_inst)
{
    app_usbd_class_iface_conf_t const * class_iface;
    class_iface = app_usbd_class_iface_get(p_inst, APP_USBD_MIDI_STREAMING_IFACE_IDX);

    app_usbd_class_ep_conf_t const * ep_cfg;
    ep_cfg = app_usbd_class_iface_ep_get(class_iface, APP_USBD_MIDI_STREAMING_EP_OUT_IDX);

    return app_usbd_class_ep_address_get(ep_cfg);
}

/**
 * @brief Auxiliary function to access midi in endpoint address.
 *
 * @param[in] p_inst Class instance data.
 *
 * @return IN endpoint address.
 */
static inline nrf_drv_usbd_ep_t ep_in_addr_get(app_usbd_class_inst_t const * p_inst)
{
    app_usbd_class_iface_conf_t const * class_iface;
    class_iface = app_usbd_class_iface_get(p_inst, APP_USBD_MIDI_STREAMING_IFACE_IDX);

    app_usbd_class_ep_conf_t const * ep_cfg;
    ep_cfg = app_usbd_class_iface_ep_get(class_iface, APP_USBD_MIDI_STREAMING_EP_IN_IDX);

    return app_usbd_class_ep_address_get(ep_cfg);
}


size_t midi_buffer_get(app_usbd_midi_t const *p_midi, uint8_t **p_buf)
{
    size_t len = 4;
    size_t len_total = 0;
    uint8_t buf[64];
    while ((len != 0) && (len_total < 64))
    {
        nrf_ringbuf_cpy_get(p_midi->specific.inst.p_in_buf, 
        (buf + len_total), &len);

        len_total += len;
    }
    *p_buf = buf;
    return len_total;
    
}

/**
 * @brief Class specific endpoint transfer handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 */
static ret_code_t midi_endpoint_ev(app_usbd_class_inst_t const *  p_inst,
                                   app_usbd_complex_evt_t const * p_event)
{
    app_usbd_midi_t const * p_midi     = midi_get(p_inst);
    app_usbd_midi_ctx_t * p_midi_ctx = midi_ctx_get(p_midi);
    ret_code_t ret = NRF_ERROR_NOT_SUPPORTED;


    if (NRF_USBD_EPIN_CHECK(p_event->drv_evt.data.eptransfer.ep))
    {
        switch (p_event->drv_evt.data.eptransfer.status)
        {
            case NRF_USBD_EP_OK:
            {
                size_t len = 4;
                size_t len_total = 0;
                uint8_t p_buffer[64];

                while ((len != 0) && (len_total < 64))
                {
                    nrf_ringbuf_cpy_get(p_midi->specific.inst.p_in_buf, 
                    (p_buffer + len_total), &len);

                    len_total += len;
                }

                if (len_total) {
                    NRF_DRV_USBD_TRANSFER_IN(transfer, p_buffer, len_total);
                    app_usbd_ep_transfer(NRF_DRV_USBD_EPIN1, &transfer);                
                } else {
                    p_midi_ctx->sending = false;
                }

                user_event_handler(p_inst, APP_USBD_MIDI_USER_EVT_TX_DONE);
                return NRF_SUCCESS;
            }

                
            case NRF_USBD_EP_ABORTED:
                return NRF_SUCCESS;
            default:
                return NRF_ERROR_INTERNAL;
        }
    }

    if (NRF_USBD_EPOUT_CHECK(p_event->drv_evt.data.eptransfer.ep))
    {
        switch (p_event->drv_evt.data.eptransfer.status)
        {
            case NRF_USBD_EP_OK:
            {   
                uint8_t *buf;
                uint8_t *buf_temp;
                uint8_t len;
                uint8_t cin;
                uint8_t cable;
                app_usbd_midi_msg_t msg;

                nrf_drv_usbd_handler_desc_t handler_desc = {
                    .handler.consumer = midi_consumer,
                    .p_context        = p_midi_ctx
                };

                buf_temp = p_midi_ctx->rx_transfer[p_midi_ctx->rx_buf].data;
                len = p_midi_ctx->rx_transfer[p_midi_ctx->rx_buf].len;
                app_usbd_ep_handled_transfer(NRF_DRV_USBD_EPOUT1, &handler_desc);

                for (size_t i = 0; i < len; i += 4)
                {
                    buf = buf_temp + i;
                    cin = (*buf) & 0xF;
                    cable = (*buf) >> 4;

                    switch (cin)
                    {
                    case 0x4:
                    {
                        msg.p_data = p_midi_ctx->sysex[cable].p_data,
                        msg.len = p_midi_ctx->sysex[cable].pos;
                        
                        if (((p_midi_ctx->sysex[cable].left < 3) && msg.p_data != NULL) || 
                            (*(buf + 1) == 0xF0 && msg.p_data == NULL)) 
                        {
                            user_rx_handler(p_inst,
                                    APP_USBD_MIDI_SYSEX_BUF_REQ,
                                    cable, 
                                    &msg);
  
                            p_midi_ctx->sysex[cable].pos = 0;
                            p_midi_ctx->sysex[cable].left = msg.len;
                            p_midi_ctx->sysex[cable].p_data = msg.p_data;
                        }

                        if (p_midi_ctx->sysex[cable].p_data != NULL)
                        {
                            memcpy(p_midi_ctx->sysex[cable].p_data + p_midi_ctx->sysex[cable].pos, buf + 1, 3); 
                            p_midi_ctx->sysex[cable].pos += 3;
                            p_midi_ctx->sysex[cable].left -= 3;
                        }
                        break;
                    }
                    case 0x5:
                        if (*(buf + 1) == 0xF6)
                        {
                            msg.p_data = buf + 1;
                            msg.len = 1;
                            user_rx_handler(p_inst, APP_USBD_MIDI_RX_DONE, cable, &msg);
                            break;
                        }
                    case 0x6:
                    case 0x7:
                        if (p_midi_ctx->sysex[cable].left < cin-4 && p_midi_ctx->sysex[cable].p_data != NULL) 
                        {
                            msg.p_data = p_midi_ctx->sysex[cable].p_data,
                            msg.len = p_midi_ctx->sysex[cable].pos;

                            user_rx_handler(p_inst, APP_USBD_MIDI_SYSEX_BUF_REQ, cable, &msg);

                            p_midi_ctx->sysex[cable].pos = 0;
                            p_midi_ctx->sysex[cable].left = msg.len;
                            p_midi_ctx->sysex[cable].p_data = msg.p_data;
                        }

                        if (p_midi_ctx->sysex[cable].p_data != NULL)
                        {
                            memcpy(p_midi_ctx->sysex[cable].p_data + p_midi_ctx->sysex[cable].pos, buf + 1, cin-4);
                            p_midi_ctx->sysex[cable].pos += cin-4;

                            msg.p_data = p_midi_ctx->sysex[cable].p_data,
                            msg.len = p_midi_ctx->sysex[cable].pos;

                            user_rx_handler(p_inst, APP_USBD_MIDI_SYSEX_RX_DONE, cable, &msg);

                            p_midi_ctx->sysex[cable].pos = 0;
                            p_midi_ctx->sysex[cable].left = 0;
                            p_midi_ctx->sysex[cable].p_data = NULL;
                        }
                        break;
                    case 0xF:
                        msg.p_data = buf + 1;
                        msg.len = 1;
                        user_rx_handler(p_inst, APP_USBD_MIDI_RX_DONE, cable, &msg);
                        break;
                    case 0x2:
                    case 0xC:
                    case 0xD:
                        msg.p_data = buf + 1;
                        msg.len = 2;
                        user_rx_handler(p_inst, APP_USBD_MIDI_RX_DONE, cable, &msg);
                        break;
                    default:
                        msg.p_data = buf + 1;
                        msg.len = 3;
                        user_rx_handler(p_inst, APP_USBD_MIDI_RX_DONE, cable, &msg);
                        break;
                    }
                }
                return NRF_SUCCESS;
                
            }
            case NRF_USBD_EP_WAITING:
            case NRF_USBD_EP_ABORTED:
                return NRF_SUCCESS;
            default:
                return NRF_ERROR_INTERNAL;
        }
    }

    return ret;
}

/**
 * @brief @ref app_usbd_class_methods_t::event_handler
 */
static ret_code_t midi_event_handler(
    app_usbd_class_inst_t const  * p_inst,
    app_usbd_complex_evt_t const * p_event)
{
    ASSERT(p_inst != NULL);
    ASSERT(p_event != NULL);

    ret_code_t ret = NRF_SUCCESS;
    switch (p_event->app_evt.type)
    {
        case APP_USBD_EVT_DRV_RESET:
            break;

        case APP_USBD_EVT_DRV_SETUP:
            ret = setup_event_handler(p_inst, (app_usbd_setup_evt_t const *)p_event);
            break;

        case APP_USBD_EVT_DRV_EPTRANSFER:
            ret = midi_endpoint_ev(p_inst, p_event);
            break;

        case APP_USBD_EVT_DRV_SUSPEND:
            break;

        case APP_USBD_EVT_DRV_RESUME:
            break;

        case APP_USBD_EVT_INST_APPEND:
            break;

        case APP_USBD_EVT_INST_REMOVE:
            break;

        case APP_USBD_EVT_STARTED:
            break;

        case APP_USBD_EVT_STOPPED:
            break;

        case APP_USBD_EVT_STATE_CHANGED:
            break;

        default:
            ret = NRF_ERROR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

static size_t midi_get_descriptor_size(app_usbd_class_inst_t const * p_inst)
{
    app_usbd_midi_t const * p_midi = midi_get(p_inst);

    if (p_midi->specific.inst.p_midi_dsc == NULL)
    {
        return 0;
    }

    return p_midi->specific.inst.p_midi_dsc->size;
}

static size_t midi_get_descriptor_data(app_usbd_class_inst_t const * p_inst,
                                               uint32_t                      cur_byte)
{
    app_usbd_midi_t const * p_midi = midi_get(p_inst);

    return p_midi->specific.inst.p_midi_dsc->p_data[cur_byte];
}

/**
 * @brief @ref app_usbd_class_methods_t::feed_descriptors
 */
static bool midi_feed_descriptors(app_usbd_class_descriptor_ctx_t * p_ctx,
                                   app_usbd_class_inst_t const     * p_inst,
                                   uint8_t                         * p_buff,
                                   size_t                            max_size)
{
    static uint8_t ifaces   = 0;
    ifaces = app_usbd_class_iface_count_get(p_inst);
    ASSERT(ifaces == 2);
    app_usbd_midi_t const * p_midi = midi_get(p_inst);

    APP_USBD_CLASS_DESCRIPTOR_BEGIN(p_ctx, p_buff, max_size);

    /* CONTROL INTERFACE DESCRIPTOR */
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x09); // bLength
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_DESCRIPTOR_INTERFACE); // bDescriptorType = Interface

    static app_usbd_class_iface_conf_t const * p_cur_iface = NULL;
    p_cur_iface = app_usbd_class_iface_get(p_inst, 0);

    APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_iface_number_get(p_cur_iface)); // bInterfaceNumber
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00); // bAlternateSetting
    APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_iface_ep_count_get(p_cur_iface)); // bNumEndpoints
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_CLASS); // bInterfaceClass = Audio
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_SUBCLASS_AUDIOCONTROL); // bInterfaceSubclass (Audio Control)
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_CLASS_PROTOCOL_UNDEFINED); // bInterfaceProtocol
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00); // iInterface

    /* HEADER INTERFACE */
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x09); // bLength
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_DESCRIPTOR_INTERFACE); // bDescriptorType = Audio Interfaces
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_AC_IFACE_SUBTYPE_HEADER); // bDescriptorSubtype = Header
    APP_USBD_CLASS_DESCRIPTOR_WRITE(LSB_16(0x0100)); // bcdADC LSB
    APP_USBD_CLASS_DESCRIPTOR_WRITE(MSB_16(0x0100)); // bcdADC MSB

    static uint16_t header_desc_len = 0;
    header_desc_len = 9;  //!! decriptor size
    APP_USBD_CLASS_DESCRIPTOR_WRITE(LSB_16(header_desc_len)); // wTotalLength LSB
    APP_USBD_CLASS_DESCRIPTOR_WRITE(MSB_16(header_desc_len)); // wTotalLength MSB
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x01);                    // bInCollection
    APP_USBD_CLASS_DESCRIPTOR_WRITE(1); // baInterfaceNr(1) //!! interface number hardcoded

    // /* INPUT TERMINAL DESCRIPTOR */
    static uint32_t cur_byte        = 0;


    p_cur_iface++;

    /* STREAM INTERFACE DESCRIPTOR ALT 0 */
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x09); // bLength
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_DESCRIPTOR_INTERFACE); // bDescriptorType = Interface
    APP_USBD_CLASS_DESCRIPTOR_WRITE(1); // bInterfaceNumber //!! Hardcoded
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00); // bAlternateSetting
    APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_iface_ep_count_get(p_cur_iface)); // bNumEndpoints
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_CLASS); // bInterfaceClass = Audio
    APP_USBD_CLASS_DESCRIPTOR_WRITE(p_midi->specific.inst.type_streaming); // bInterfaceSubclass (Audio Control)
    APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_AUDIO_CLASS_PROTOCOL_UNDEFINED); // bInterfaceProtocol
    APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00); // iInterface

    if (p_midi->specific.inst.type_streaming == APP_USBD_AUDIO_SUBCLASS_MIDISTREAMING) {
            static size_t midi_desc_size = 0;
        midi_desc_size = midi_get_descriptor_size(p_inst);

        for (cur_byte = 0; cur_byte < midi_desc_size; cur_byte++)
        {
            APP_USBD_CLASS_DESCRIPTOR_WRITE(midi_get_descriptor_data(p_inst, cur_byte));
        }
    }

    APP_USBD_CLASS_DESCRIPTOR_END();
}

const app_usbd_class_methods_t app_usbd_midi_class_methods = {
    .event_handler       = midi_event_handler,
    .feed_descriptors    = midi_feed_descriptors,
    .iface_select        = iface_select,
    .iface_deselect      = iface_deselect,
    .iface_selection_get = iface_selection_get,
};

ret_code_t app_usbd_midi_send_raw(app_usbd_midi_t const * p_midi,
                                  const void *        p_buf,
                                  size_t              len)
{
    app_usbd_midi_ctx_t * p_midi_ctx = midi_ctx_get(p_midi);

    #if (APP_USBD_CONFIG_EVENT_QUEUE_ENABLE == 0)
    CRITICAL_REGION_ENTER();
    #endif // (APP_USBD_CONFIG_EVENT_QUEUE_ENABLE == 0)

    nrf_ringbuf_cpy_put(p_midi->specific.inst.p_in_buf, p_buf, &len);

    if(!p_midi_ctx->sending) 
    {
        p_midi_ctx->sending = true;
        len = 4;
        size_t len_total = 0;
        uint8_t p_buffer[64];
        
        while ((len != 0) && (len_total < 64))
        {
            nrf_ringbuf_cpy_get(p_midi->specific.inst.p_in_buf, 
            (p_buffer + len_total), &len);

            len_total += len;
        }

        if (len_total) 
        {
            NRF_DRV_USBD_TRANSFER_IN(transfer, p_buffer, len_total);
            app_usbd_ep_transfer(NRF_DRV_USBD_EPIN1, &transfer);        
        } 
        else 
        {
            p_midi_ctx->sending = false;
        }
    }  

    #if (APP_USBD_CONFIG_EVENT_QUEUE_ENABLE == 0)
    CRITICAL_REGION_EXIT();
    #endif // (APP_USBD_CONFIG_EVENT_QUEUE_ENABLE == 0)

    return NRF_SUCCESS;
}

ret_code_t app_usbd_midi_write(app_usbd_midi_t const *  p_midi,
                               uint8_t                  cable, 
                               uint8_t *                p_buf,
                               size_t                   len)
{
    uint8_t status = *p_buf;
    uint8_t cin;
    uint8_t m_tx_buffer[4];

    if (len > 3) {
        return NRF_ERROR_INVALID_DATA;
    }

    if (status >> 5 == 0b110) 
    {
        cin = 0x2;
    } 
    else if (status == 0b11110010) 
    {
        cin = 0x3;
    } 
    else if ((status >> 2) == 0b111101) 
    {
       cin = 0x5;
    }
    else 
    {
        cin = status >> 4;
    }

    m_tx_buffer[0] = (cable << 4) + cin;
    for (size_t i = 0; i < len; i++)
    {
        m_tx_buffer[i+1] = *(p_buf + i);
    }

    return app_usbd_midi_send_raw(p_midi, m_tx_buffer, 4);
}

ret_code_t app_usbd_midi_sysex_write(app_usbd_midi_t const *  p_midi,
                               uint8_t                  cable, 
                               uint8_t *                p_buf,
                               size_t                   len)
{
    uint8_t sysex_buf[64];
    size_t sysex_pos = 0;
    size_t pos = 0;
    uint8_t left_to_send = 0;

    while (pos < len)
    {
        left_to_send =  (len - pos);
        
        if (left_to_send > 3) 
        {
            sysex_buf[sysex_pos] = 0x4;
        } 
        else 
        {
            sysex_buf[sysex_pos] = 4 + left_to_send;
        }
        sysex_pos++;
        for (size_t i = 0; i < left_to_send; i++)
        {
            sysex_buf[sysex_pos] = *(p_buf + pos);
            pos ++;
            sysex_pos ++;
            
            if(i == 2) 
            {
                break;
            }
        }
    }
    sysex_pos += 3-left_to_send;
    return app_usbd_midi_send_raw(p_midi, sysex_buf, sysex_pos);
}

/** @} */

#endif //NRF_MODULE_ENABLED(APP_USBD_AUDIO)

