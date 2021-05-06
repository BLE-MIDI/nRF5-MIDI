#include "nordic_common.h"
#include "app_util.h"
#include "nrf_log.h"
#include "nrf_error.h"


/**
 * String config descriptor
 */
#define USBD_STRING_LANG_IX  0x00
#define USBD_STRING_LANG \
    0x04,         /* length of descriptor                   */\
    0x03,         /* descriptor type                        */\
    0x09,         /*                                        */\
    0x04          /* Supported LangID = 0x0409 (US-English) */




#define USBD_STRING_MANUFACTURER_IX  0x01
#define USBD_STRING_MANUFACTURER \
    42,           /* length of descriptor (? bytes)   */\
    0x03,         /* descriptor type                  */\
    'N', 0x00,    /* Define Unicode String "Nordic Semiconductor  */\
    'o', 0x00, \
    'r', 0x00, \
    'd', 0x00, \
    'i', 0x00, \
    'c', 0x00, \
    ' ', 0x00, \
    'S', 0x00, \
    'e', 0x00, \
    'm', 0x00, \
    'i', 0x00, \
    'c', 0x00, \
    'o', 0x00, \
    'n', 0x00, \
    'd', 0x00, \
    'u', 0x00, \
    'c', 0x00, \
    't', 0x00, \
    'o', 0x00, \
    'r', 0x00,  

#define USBD_STRING_PRODUCT_IX  0x02 
#define USBD_STRING_PRODUCT \
    16,           /* length of descriptor (? bytes)         */\
    0x03,         /* descriptor type                        */\
    'E', 0x00,    /* generic unicode string for all devices */\
    'x', 0x00, \
    'a', 0x00, \
    'm', 0x00, \
    'p', 0x00, \
    'l', 0x00, \
    'e', 0x00 






#define USBD_MIDI_DEVICE_DESCRIPTOR                                               \
    0x12,           /* bLength              | size of descriptor                */\
    0x01,           /* bDescriptorType      | descriptor type                   */\
    0x00, 0x02,     /* bcdUSB               | USB spec release (ver 2.0)        */\
    0x00,           /* bDeviceClass         | class code                        */\
    0x00,           /* bDeviceSubClass      | device sub-class                  */\
    0x00,           /* bDeviceProtocol      | device protocol                   */\
    0x40,           /* bMaxPacketSize0      | maximum packet size  (This is different from spec, experiment wit this one)*/\
    0x15, 0x19,     /* idVendor             | Vendor ID                         */\
    0x0A, 0x52,     /* idProduct            | Product ID.                       */\
    0x01, 0x01,     /* bcdDevice            | Device Release Code               */\
    0x01,           /* iManufacturer        | index of manufacturer string      */\
    0x02,           /* iProduct             | index of product string           */\
    0x00,           /* iSerialNumber        | Serial Number string              */\
    0x01            /* bNumConfigurations   | number of configurations          */


#define USBD_MIDI_CONFIG_DESCRIPTOR                                               \
    0x09,         /* bLength                | size of descriptor                */\
    0x02,         /* bDescriptorType        | descriptor type (CONFIGURATION)   */\
    0x65, 0x00,   /* wTotalLength           | total length of descriptors       */\
    0x02,         /* bNumInterfaces         | Two interfaces.                   */\
    0x01,         /* bConfigurationValue    | ID of this configuration.         */\
    0x00,         /* iConfiguration         | index of string Configuration     */\
    0x80,         /* bmAttributes           | Bus powered, remote wakeup        */\
    0x64          /* MaxPower               | maximum power in steps of 2mA     */

#define USBD_STANDARD_AUDIO_CONTROL_INTERFACE_DESCRIPTOR                          \
    0x09,         /* bLength                | Size of this descriptor           */\
    0x04,         /* bDescriptorType        | descriptor type (INTERFACE)       */\
    0x00,         /* bInterfaceNumber       | Index of this interface.          */\
    0x00,         /* bAlternateSetting      | Index of this setting.            */\
    0x00,         /* bNumEndpoints          | number of endpoints               */\
    0x01,         /* bInterfaceClass        | interface class                   */\
    0x01,         /* bInterfaceSubClass     | interface sub-class               */\
    0x00,         /* bInterfaceProtocol     | interface protocol                */\
    0x00          /* iInterface             | interface string index            */

#define USBD_CLASS_SPECIFIC_AUDIO_CONTROL_INTERFACE_DESCRIPTOR                    \
    0x09,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x01,         /* bDescriptorSubtype     | HEADER subtype                    */\
    0x00, 0x01,   /* BcdADC                 | Revision of class spec            */\
    0x09, 0x00,   /* wTotalLength           | Total size of class specific desc */\
    0x01,         /* bInCollection          | Number of streaming interfaces    */\
    0x01          /* baInterfaceNr(1)       | streaming interface number        */\

#define USBD_STANDARD_MIDI_STREAMING_INTERFACE_DESCRIPTOR                         \
    0x09,         /* bLength                | Size of this descriptor           */\
    0x04,         /* bDescriptorType        | descriptor type (INTERFACE)       */\
    0x01,         /* bInterfaceNumber       | Index of this interface.          */\
    0x00,         /* bAlternateSetting      | Index of this setting.            */\
    0x02,         /* bNumEndpoints          | number of endpoints               */\
    0x01,         /* bInterfaceClass        | interface class                   */\
    0x03,         /* bInterfaceSubclass     | interface subclass                */\
    0x00,         /* bInterfaceProtocol     | interface protocol                */\
    0x00          /* iInterface             | interface string index            */


#define USBD_CLASS_SPECIFIC_MIDI_STREAMING_INTERFACE_DESCRIPTOR                   \
    0x07,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x01,         /* bDescriptorSubtype     | HEADER subtype                    */\
    0x00, 0x01,   /* BcdADC                 | Revision of class spec            */\
    0x41, 0x00    /* wTotalLength           | Total size of CS descriptors.       */

#define USBD_MIDI_EMBEDDED_IN_JACK_DESCRIPTOR                                     \
    0x06,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x02,         /* bDescriptorSubtype     | MIDI_IN_JACK subtype              */\
    0x01,         /* bJackType              | EMBEDDED                          */\
    0x01,         /* bJackID                | ID of this Jack                   */\
    0x00          /* iJack                  | Unused.                           */

#define USBD_MIDI_EXTERNAL_IN_JACK_DESCRIPTOR                                     \
    0x06,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x02,         /* bDescriptorSubtype     | MIDI_IN_JACK subtype              */\
    0x02,         /* bJackType              | EXTERNAL                          */\
    0x02,         /* bJackID                | ID of this Jack                   */\
    0x00          /* iJack                  | Unused.                           */

#define USBD_MIDI_EMBEDDED_OUT_JACK_DESCRIPTOR                                    \
    0x09,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x03,         /* bDescriptorSubtype     | MIDI_OUT_JACK subtype             */\
    0x01,         /* bJackType              | EMBEDDED                          */\
    0x03,         /* bJackID                | ID of this Jack                   */\
    0x01,         /* bNrInputPins           | Number of Input Pins of this Jack */\
    0x02,         /* BaSourceID(1)          | ID of the Entity                  */\
    0x01,         /* BaSourcePin(1)         | Output Pin number of the Entity   */\
    0x00          /* iJack                  | Unused                            */

#define USBD_MIDI_EXTERNAL_OUT_JACK_DESCRIPTOR                                    \
    0x09,         /* bLength                | length of descriptor              */\
    0x24,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x03,         /* bDescriptorSubtype     | MIDI_OUT_JACK subtype             */\
    0x02,         /* bJackType              | EXTERNAL                          */\
    0x04,         /* bJackID                | ID of this Jack                   */\
    0x01,         /* bNrInputPins           | Number of Input Pins of this Jack */\
    0x01,         /* BaSourceID(1)          | ID of the Entity                  */\
    0x01,         /* BaSourcePin(1)         | Output Pin number of the Entity   */\
    0x00          /* iJack                  | Unused                            */

#define USBD_MIDI_STANDARD_BULK_OUT_ENDPOINT_DESCRIPTOR                           \
    0x09,         /* bLength                | length of descriptor              */\
    0x05,         /* bDescriptorType        | descriptor type (ENDPOINT)        */\
    0x01,         /* bEndpointAddress       | OUT Endpoint 1.                   */\
    0x02,         /* bmAttributes           | Bulk, not shared.                 */\
    0x40, 0x00,   /* wMaxPacketSize         | 64 bytes per packet               */\
    0x00,         /* bInterval              | Ignored for Bulk. Set to zero.    */\
    0x00,         /* bRefresh               | Unused.                           */\
    0x00          /* bSynchAddress          | Unused                            */

#define USBD_MIDI_CLASS_SPECIFIC_BULK_OUT_ENDPOINT_DESCRIPTOR                     \
    0x05,         /* bLength                | length of descriptor              */\
    0x25,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x01,         /* bDescriptorSubtype     | MS_GENERAL subtype                */\
    0x01,         /* bNumEmbMIDIJack        | Number of embedded MIDI IN Jacks  */\
    0x01          /* BaAssocJackID(1)       | ID of the Embedded MIDI IN Jack.  */


#define USBD_MIDI_STANDARD_BULK_IN_ENDPOINT_DESCRIPTOR                            \
    0x09,         /* bLength                | length of descriptor              */\
    0x05,         /* bDescriptorType        | descriptor type (ENDPOINT)        */\
    0x81,         /* bEndpointAddress       | IN Endpoint 1.                    */\
    0x02,         /* bmAttributes           | Bulk, not shared.                 */\
    0x40, 0x00,   /* wMaxPacketSize         | 64 bytes per packet               */\
    0x00,         /* bInterval              | Ignored for Bulk. Set to zero.    */\
    0x00,         /* bRefresh               | Unused.                           */\
    0x00          /* bSynchAddress          | Unused                            */

#define USBD_MIDI_CLASS_SPECIFIC_BULK_IN_ENDPOINT_DESCRIPTOR                      \
    0x05,         /* bLength                | length of descriptor              */\
    0x25,         /* bDescriptorType        | descriptor type (CS_INTERFACE)    */\
    0x01,         /* bDescriptorSubtype     | MS_GENERAL subtype                */\
    0x01,         /* bNumEmbMIDIJack        | Number of embedded MIDI OUT Jacks */\
    0x03          /* BaAssocJackID(1)       | ID of the Embedded MIDI OUT Jack  */

     
     
     