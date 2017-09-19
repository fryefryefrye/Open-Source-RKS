/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 2513 $
 */

/** @file
 * @brief Interface functions for the on-chip radio transceiver.
 */

/** @defgroup hal_nrf24l01p_hal_nrf RF tranceiver (hal_nrf)
 * @{
 * @ingroup hal_nrf24l01p
 *
 * This is the nRF24L01+ transceiver used in several Nordic Semiconductor devices.
 * The transceiver is set up and controlled via an internal SPI interface on the chip.
 * The HAL for the radio transceiver hides this SPI interface from the programmer.
 *
 * The nRF24LE1 uses the same 2.4GHz GFSK RF transceiver with embedded protocol engine
 * (Enhanced ShockBurst&tm;) that is found in the nRF24L01+ single chip RF Transceiver.
 *
 * The RF Transceiver module is configured and operated through the RF transceiver map.
 * This register map is accessed by the MCU through a dedicated on-chip Serial
 * Peripheral interface (SPI) and is available in all power modes of the RF Transceiver
 * module. The register map contains all configuration registers in the RF Transceiver
 * and is accessible in all operation modes of the transceiver. The radio transceiver
 * HAL hides this register map and the usage of the internal SPI.
 *
 * This HAL module contains setup functions for configurating the radio; operation functions
 * for controlling the radio when active and for sending and receiving data; and test
 * functions for setting the radio in test modes.
 */

#ifndef HAL_NRF_H__
#define HAL_NRF_H__

#include <stdint.h>
#include <stdbool.h>

#include "hal_nrf_reg.h"
#include "hal_nrf_hw.h"

/** @name Setup */
//@{
/* Setup function prototypes */

/** Enable or disable interrupt for radio.
 * Use this function to enable or disable
 * one of the interrupt sources for the radio.
 * This function only changes state for selected
 * int_type, the rest of the interrupt sources
 * are left unchanged.
 *
 * @param int_source Radio interrupt Source.
 * @param irq_state Enable or Disable.
*/
void hal_nrf_set_irq_mode(hal_nrf_irq_source_t int_source, bool irq_state);

/* For the obsolete nRF24L01 it is necessary to issue an activate command before the
 * features enabled by the FEATURE register can be used. For nRF24L01+ these features
 * are by default enabled.
*/
void hal_nrf_activate_features(void);

/** Enables the dynamic packet length
 * @param enable Whether enable or disable dynamic packet length
 */
void hal_nrf_enable_dynamic_payload(bool enable);

/** Enables the ACK payload feature
 * @param enable Whether to enable or disable ACK payload
 */
void hal_nrf_enable_ack_payload(bool enable);

/** Enables the dynamic ack feature
 * @param enable Whether to enable or disable Dynamic ACK
 */
void hal_nrf_enable_dynamic_ack(bool enable);

/** Function for enabling dynmic payload size.
 * The input parameter contains is a byte where the bit values tells weather the
 * pipe should use dynamic payload size. For example if bit 0 is set then
 * pipe 0 will accept dynamic payload size.
 * @param setup Byte value telling for which pips(s) to enable dynamic payload size
 */
void hal_nrf_setup_dynamic_payload(uint8_t setup);

/** Reads the payload width of the received ack payload
 * @return Payload width of the received ack payload
 */
uint8_t hal_nrf_read_rx_payload_width(void);

/** Write ack payload
 * Writes the payload that will be transmitted with the ack on the given pipe.
 * @param pipe Pipe that transmits the payload
 * @param tx_pload Pointer to the payload data
 * @param length Size of the data to transmit
 */
void hal_nrf_write_ack_payload(uint8_t pipe, const uint8_t *tx_pload, uint8_t length);

/** Read then clears all interrupt flags.
 * Use this function to get the interrupt flags and clear them in the same operation.
 * Reduced radio interface activity and speed optimized.
 *
 * @return  Interrupt_flags
 * @retval 0x10 Max Retransmit interrupt
 * @retval 0x20 TX Data sent interrupt
 * @retval 0x40 RX Data received interrupt
*/
uint8_t hal_nrf_get_clear_irq_flags(void);

uint8_t hal_nrf_clear_irq_flags_get_status(void);

/** Clear one selected interrupt flag.
 * Use this function to clear one @a spesific interrupt flag.
 * Other interrupt flags are left unchanged.
 *
 * @param int_source Interrupt source of which flag to clear
*/
void hal_nrf_clear_irq_flag(hal_nrf_irq_source_t int_source);

/** Set the CRC mode used by the radio.
 * Use this function to set the CRC mode; CRC disabled, 1 or 2 bytes.
 *
 * @param crc_mode CRC mode to use
*/
void hal_nrf_set_crc_mode(hal_nrf_crc_mode_t crc_mode);

/** Open radio pipe(s) and enable/ disable auto acknowledge.
 * Use this function to open one or all pipes,
 * with or without auto acknowledge.
 *
 * @param pipe_num Radio pipe to open
 * @param auto_ack Auto_Ack ON/OFF
 * @see hal_nrf_address
*/
void hal_nrf_open_pipe(hal_nrf_address_t pipe_num, bool auto_ack);

/** Close radio pipe(s).
 * Use this function to close one pipe or all pipes.
 *
 * @param pipe_num Pipe# number to close
*/
void hal_nrf_close_pipe(hal_nrf_address_t pipe_num);

/** Set radio's RX address and TX address.
 * Use this function to set a RX address, or to set the TX address.
 * Beware of the difference for single and multibyte address registers.
 *
 * @param address Which address to set
 * @param *addr Buffer from which the address is stored in
*/
void hal_nrf_set_address(const hal_nrf_address_t address, const uint8_t *addr);

/** Get address for selected pipe.
 * Use this function to get address for selected pipe.
 *
 *
 * @param address Which address to get, Pipe- or TX-address
 * @param *addr buffer in which address bytes are written.
 * <BR><BR>For pipes containing only LSB byte of address, this byte is returned
 * in the<BR> *addr buffer.
 *
 * @return Numbers of bytes copied to addr
*/
uint8_t hal_nrf_get_address (uint8_t address, uint8_t *addr);

/** Set auto acknowledge parameters.
 * Use this function to set retransmit and retransmit delay
 * parameters.
 *
 * @param retr Number of retransmit, 0 equ retransmit OFF
 * @param delay Retransmit delay in µs. Must be a
*/
void hal_nrf_set_auto_retr(uint8_t retr, uint16_t delay);

/** Set radio's address width.
 * Use this function to define the radio's address width,
 * referes to both RX and TX.
 *
 * @param address_width Address with in bytes
*/
void hal_nrf_set_address_width(hal_nrf_address_width_t address_width);

/** Gets the radio's address width.
 *
 * @return Address width
 */
uint8_t hal_nrf_get_address_width (void);

/** Set payload width for selected pipe.
 * Use this function to set the number of bytes expected
 * on a selected pipe.
 *
 * @param pipe_num Pipe number to set payload width for
 * @param pload_width number of bytes expected
*/
void hal_nrf_set_rx_payload_width(uint8_t pipe_num, uint8_t pload_width);

/** Read current interrupt mode for selected interrupt source.
 * Use this function to get the interrupt source's mode,
 * either enabled or disabled.
 *
 * @param int_source Interrupt source to get mode from
 *
 * @return Interrupt Mode
 * @retval FALSE Interrupt disabled
 * @retval TRUE Interrupt enabled
*/
bool hal_nrf_get_irq_mode(uint8_t int_source);

/** Read all interrupt flags.
 * Use this function to get the interrupt flags. This function is similar
 * to hal_nrf_get_clear_irq_flags with the exception that it does <I><B>NOT</B></I> clear
 * the irq_flags.
 *
 * @return Interrupt_flags
 * @retval 0x10 Max Retransmit interrupt
 * @retval 0x20 TX Data sent interrupt
 * @retval 0x40 RX Data received interrupt
*/
uint8_t hal_nrf_get_irq_flags(void);

/** Get pipe status.
 * Use this function to check status for a selected pipe.
 *
 * @param  pipe_num Pipe number to check status for
 *
 * @return Pipe_Status
 * @retval 0x00 Pipe is closed, autoack disabled
 * @retval 0x01 Pipe is open, autoack disabled
 * @retval 0x03 Pipe is open, autoack enabled
*/
uint8_t hal_nrf_get_pipe_status(uint8_t pipe_num);

/** Get auto retransmit parameters.
 * Use this function to get the auto retransmit parameters,
 * retrans count and retrans delay.
 *
 * @return AutoRetrans Parameters
 *
 * @retval UpperNibble Retransmit Delay
 * @retval LowerNibble Retransmit Count
*/
uint8_t hal_nrf_get_auto_retr_status(void);

/** Get packet lost counter
 * Use this function to get the packet(s) counter.
 *
 * @return packet lost counter
*/
uint8_t hal_nrf_get_packet_lost_ctr(void);

/** Get RX payload width for selected pipe.
 * Use this function to get the expected payload
 * width for selected ppe number.
 *
 * @param pipe_num Pipe number to get payload width for
 *
 * @return Payload_Width in bytes
*/
uint8_t hal_nrf_get_rx_payload_width(uint8_t pipe_num);
//@}

/** @name Operation */
//@{
/* Operation function prototypes */

/** Set radio's operation mode.
 * Use this function to enter PTX (primary TX)
 * or PRX (primary RX).
 *
 * @param op_mode Operation mode
*/
void hal_nrf_set_operation_mode(hal_nrf_operation_mode_t op_mode);

/** Set radio's power mode.
 * Use this function to power_up or power_down radio.
 *
 * @param pwr_mode POWER_UP or POWER_DOWN
*/
void hal_nrf_set_power_mode(hal_nrf_pwr_mode_t pwr_mode);

/** Set radio's RF channel.
 * Use this function to select which RF channel to use.
 *
 * @param channel RF channel
*/
void hal_nrf_set_rf_channel(uint8_t channel);

/** Set radio's TX output power.
 * Use this function set the radio's TX output power.
 *
 * @param power Radio's TX output power
*/
void hal_nrf_set_output_power(hal_nrf_output_power_t power);

/** Set radio's on-air datarate.
 * Use this function to select radio's on-air
 * datarate.
 *
 * @param datarate On-air datarate
*/
void hal_nrf_set_datarate(hal_nrf_datarate_t datarate);


/* Status functions prototypes */

/** Get radio's TX FIFO status.
 * Use this function to get the radio's TX
 * FIFO status.
 *
 * @return TX FIFO status
 * @retval 0x00 TX FIFO NOT empty, but NOT full
 * @retval 0x01 FIFO empty
 * @retval 0x02 FIFO full
 *
*/
uint8_t hal_nrf_get_tx_fifo_status(void);

/** Check for TX FIFO empty.
 * Use this function to check if TX FIFO
 * is empty.
 *
 * @return TX FIFO empty bit
 * @retval FALSE TX FIFO NOT empty
 * @retval TRUE TX FIFO empty
 *
*/
bool hal_nrf_tx_fifo_empty(void);

/** Check for TX FIFO full.
 * Use this function to check if TX FIFO
 * is full.
 *
 * @return TX FIFO full bit
 * @retval FALSE TX FIFO NOT full
 * @retval TRUE TX FIFO full
 *
*/
bool hal_nrf_tx_fifo_full(void);

/** Get radio's RX FIFO status.
 * Use this function to get the radio's TX
 * FIFO status.
 *
 * @return RX FIFO status
 * @retval 0x00 RX FIFO NOT empty, but NOT full
 * @retval 0x01 RX FIFO empty
 * @retval 0x02 RX FIFO full
 *
*/
uint8_t hal_nrf_get_rx_fifo_status(void);

uint8_t hal_nrf_get_fifo_status(void);

/** Check for RX FIFO empty.
 * Use this function to check if RX FIFO
 * is empty.
 *
 * Reads STATUS register to check this, not FIFO_STATUS
 *
 * @return RX FIFO empty bit
 * @retval FALSE RX FIFO NOT empty
 * @retval TRUE RX FIFO empty
 *
*/
bool hal_nrf_rx_fifo_empty(void);

/** Check for RX FIFO full.
 * Use this function to check if RX FIFO
 * is full.
 *
 * @return RX FIFO full bit
 * @retval FALSE RX FIFO NOT full
 * @retval TRUE RX FIFO full
 *
*/
bool hal_nrf_rx_fifo_full(void);

/** Get radio's transmit attempts status.
 * Use this function to get number of retransmit
 * attempts and number of packet lost.
 *
 * @return Retransmit attempts counters
*/
uint8_t hal_nrf_get_transmit_attempts(void);

/** Get the carrier detect flag.
 * Use this function to get the carrier detect flag,
 * used to detect stationary disturbance on selected
 * RF channel.
 *
 * @return Carrier Detect
 * @retval FALSE Carrier NOT Detected
 * @retval TRUE Carrier Detected
*/
bool hal_nrf_get_carrier_detect(void);

/* Data operation prototypes */

/** Get RX data source.
 * Use this function to read which RX pipe data
 * was received on for current top level FIFO data packet.
 *
 * @return pipe number of current packet present
*/
uint8_t hal_nrf_get_rx_data_source(void);

/** Read RX payload.
 * Use this function to read top level payload
 * available in the RX FIFO.
 *
 * @param  *rx_pload pointer to buffer in which RX payload are stored
 * @return pipe number (MSB byte) and packet length (LSB byte)
*/
uint16_t hal_nrf_read_rx_payload(uint8_t *rx_pload);

/** Write TX payload to radio.
 * Use this function to write a packet of
 * TX payload into the radio.
 * <I>length</I> number of bytes, which are stored in <I>*tx_pload</I>.
 *
 * @param *tx_pload pointer to buffer in which TX payload are present
 * @param length number of bytes to write
*/
void hal_nrf_write_tx_payload(const uint8_t *tx_pload, uint8_t length);

/** Write TX payload which do not require ACK. When transmitting
 * a ACK is not required nor sent from the receiver. The payload will
 * always be assumed as "sent".
 *
 * Use this function to write a packet of
 * TX payload into the radio.
 * <I>length</I> number of bytes, which are stored in <I>*tx_pload</I>.
 *
 * @param *tx_pload pointer to buffer in which TX payload are present
 * @param length number of bytes to write
*/
void hal_nrf_write_tx_payload_noack(const uint8_t *tx_pload, uint8_t length);

/** Reuse TX payload.
 * Use this function to set that the radio is using
 * the last transmitted payload for the next packet as well.
 *
*/
void hal_nrf_reuse_tx(void);

/** Get status of reuse TX function.
 * Use this function to check if reuse TX payload is
 * activated
 *
 * @return Reuse TX payload mode
 * @retval FALSE Not activated
 * @retval TRUE Activated
*/
bool hal_nrf_get_reuse_tx_status(void);

/** Flush RX FIFO.
 * Use this function to flush the radio's
 * RX FIFO.
 *
*/
void hal_nrf_flush_rx(void);

/** Flush TX FIFO.
 * Use this function to flush the radio's
 * TX FIFO.
 *
*/
void hal_nrf_flush_tx(void);

/** No Operation command.
 * Use this function to receive the radio's
 * status register.
 *
 * @return Status register
*/
uint8_t hal_nrf_nop(void);
//@}

/** @name  Test */
//@{
/* Test functions prototypes */

/** Set radio's PLL mode.
 * Use this function to either LOCK
 * or UNLOCK the radio's PLL.
 *
 * @param pll_lock PLL locked, TRUE or FALSE
*/
void hal_nrf_set_pll_mode(bool pll_lock);

/** Set radio's LNA gain mode.
 * Use this function to either use HI
 * current or LOW current mode for the radio.
 *
 * @param lna_gain LNA gain mode
*/
void hal_nrf_set_lna_gain(bool lna_gain);

/** Enables continuous carrier transmit.
 * Use this function to enable or disable
 * continuous carrier transmission.
 * @param enable Enable continuous carrier
 */
void hal_nrf_enable_continious_wave (bool enable);

/* hal_nrf_l01 basic functions, used by all the other functions */

//@}

/** @name SPI interface */
//@{

/** Basis function, nrf_rw
 * This function is used by the basis functions
 * to exchange data with the data.
 *
 * @param value Databyte to write
 *
 * @return Databyte from radio.
*/
uint8_t hal_nrf_rw(uint8_t value);
//@}
#endif // HAL_NRF_H__
/** @} */

