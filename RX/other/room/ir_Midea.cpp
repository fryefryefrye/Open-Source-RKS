// Copyright 2017 bwze, crankyoldgit

#include "ir_Midea.h"

const uint16_t kMideaTick = 80;
const uint16_t kMideaBitMarkTicks = 7;
const uint16_t kMideaBitMark = kMideaBitMarkTicks * kMideaTick;
const uint16_t kMideaOneSpaceTicks = 21;
const uint16_t kMideaOneSpace = kMideaOneSpaceTicks * kMideaTick;
const uint16_t kMideaZeroSpaceTicks = 7;
const uint16_t kMideaZeroSpace = kMideaZeroSpaceTicks * kMideaTick;
const uint16_t kMideaHdrMarkTicks = 56;
const uint16_t kMideaHdrMark = kMideaHdrMarkTicks * kMideaTick;
const uint16_t kMideaHdrSpaceTicks = 56;
const uint16_t kMideaHdrSpace = kMideaHdrSpaceTicks * kMideaTick;
const uint16_t kMideaMinGapTicks =
kMideaHdrMarkTicks + kMideaZeroSpaceTicks + kMideaBitMarkTicks;
const uint16_t kMideaMinGap = kMideaMinGapTicks * kMideaTick;
const uint8_t kMideaTolerance = 30;  // Percent


void IRMideaAC::begin()
{
	_irsend.begin();
}

void IRMideaAC::sendMidea() {

	// Set IR carrier frequency
	_irsend.enableIROut(38);


	for (unsigned char inner_loop = 0; inner_loop < 2; inner_loop++) {
		// Header
		_irsend.mark(kMideaHdrMark);
		_irsend.space(kMideaHdrSpace);
		// Data
		//   Break data into byte segments, starting at the Most Significant
		//   Byte. Each byte then being sent normal, then followed inverted.
		//for (uint16_t i = 8; i <= nbits; i += 8) {
		//  // Grab a bytes worth of data.
		//  uint8_t segment = (data >> (nbits - i)) & 0xFF;
		//  sendData(kMideaBitMark, kMideaOneSpace, kMideaBitMark, kMideaZeroSpace,
		//           segment, 8, true);
		//}
		for (unsigned char i = 0; i <6; i ++) {
			// Grab a bytes worth of data.
			//uint8_t segment = (data >> (nbits - i)) & 0xFF;
			sendData(kMideaBitMark, kMideaOneSpace, kMideaBitMark, kMideaZeroSpace,
				remote_state[i], 8, true);
		}
		// Footer
		_irsend.mark(kMideaBitMark);
		_irsend.space(kMideaMinGap);  // Pause before repeating

	}

}






// Generic method for sending data that is common to most protocols.
// Will send leading or trailing 0's if the nbits is larger than the number
// of bits in data.
//
// Args:
//   onemark:    Nr. of usecs for the led to be pulsed for a '1' bit.
//   onespace:   Nr. of usecs for the led to be fully off for a '1' bit.
//   zeromark:   Nr. of usecs for the led to be pulsed for a '0' bit.
//   zerospace:  Nr. of usecs for the led to be fully off for a '0' bit.
//   data:       The data to be transmitted.
//   nbits:      Nr. of bits of data to be sent.
//   MSBfirst:   Flag for bit transmission order. Defaults to MSB->LSB order.
void IRMideaAC::sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
						 uint32_t zerospace, uint64_t data, uint16_t nbits,
						 bool MSBfirst) 
{
	if (nbits == 0)  // If we are asked to send nothing, just return.
		return;
	if (MSBfirst) {  // Send the MSB first.
		// Send 0's until we get down to a bit size we can actually manage.
		while (nbits > sizeof(data) * 8) {
			_irsend.mark(zeromark);
			_irsend.space(zerospace);
			nbits--;
		}
		// Send the supplied data.
		for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
			if (data & mask) {  // Send a 1
				_irsend.mark(onemark);
				_irsend.space(onespace);
			} else {  // Send a 0
				_irsend.mark(zeromark);
				_irsend.space(zerospace);
			}
	} else {  // Send the Least Significant Bit (LSB) first / MSB last.
		for (uint16_t bit = 0; bit < nbits; bit++, data >>= 1)
			if (data & 1) {  // Send a 1
				_irsend.mark(onemark);
				_irsend.space(onespace);
			} else {  // Send a 0
				_irsend.mark(zeromark);
				_irsend.space(zerospace);
			}
	}
}





// Code to emulate Midea A/C IR remote control unit.
// Warning: Consider this very alpha code.

// Initialise the object.
IRMideaAC::IRMideaAC(uint16_t pin): _irsend(pin)  
{ 
	stateReset(); 
	Temp = 26;
	Mode = kMideaACAuto;
	FanSpeed = kMideaACFanAuto;
	Power = true;
}

// Reset the state of the remote to a known good state/sequence.
void IRMideaAC::stateReset() {
	// Power On, Mode Auto, Fan Auto, Temp = 26C
	remote_state[0] = 0xB2;
	remote_state[1] = 0x4D;
	remote_state[2] = 0x1F;
	remote_state[3] = 0xE0;
	remote_state[4] = 0xD8;
	remote_state[5] = 0x27;
}



// Send the current desired state to the IR LED.
void IRMideaAC::send() {
	calc();  // Ensure correct checksum before sending.
	sendMidea();
}


// Return a pointer to the internal state date of the remote.
unsigned char * IRMideaAC::getRaw() {
	calc();
	return remote_state;
}

// Override the internal state with the new state.
void IRMideaAC::setRaw(unsigned char *  newState) {
	for (unsigned char i = 0; i<6 ; i++)
	{
		remote_state[i] = newState[i];
	}

}

// Set the requested power state of the A/C.
void IRMideaAC::setPower(const bool state) {
	Power = state;
}

// Return the requested power state of the A/C.
bool IRMideaAC::getPower() { return Power; }


void IRMideaAC::setTemp(const uint8_t temp) 
{
	Temp = temp;
	if (Temp < 17)
	{
		Temp = 17;
	} 
	if(Temp > 30)
	{
		Temp = 30;
	}

}


uint8_t IRMideaAC::getTemp() 
{
	return Temp;
}

// Set the speed of the fan,
// 1-3 set the speed, 0 or anything else set it to auto.
void IRMideaAC::setFan(const uint8_t fan) {
	unsigned char new_fan;
	switch (fan) {
	case kMideaACFanLow:
	case kMideaACFanMed:
	case kMideaACFanHigh:
		FanSpeed = fan;
		break;
	default:
		FanSpeed = kMideaACFanAuto;
	}
}

// Return the requested state of the unit's fan.
uint8_t IRMideaAC::getFan() { 
	return 0;//(remote_state >> 35) & 0b111; 
}

// Get the requested climate operation mode of the a/c unit.
// Returns:
//   A uint8_t containing the A/C mode.
uint8_t IRMideaAC::getMode() { 
	return 0;//((remote_state >> 32) & 0b111); 
}

// Set the requested climate operation mode of the a/c unit.
void IRMideaAC::setMode(const uint8_t mode) {
	// If we get an unexpected mode, default to AUTO.
	switch (mode) {
	case kMideaACAuto:
	case kMideaACCool:
	case kMideaACHeat:
	case kMideaACDry:
		Mode = mode;
		break;


		//case kMideaACFan:
		//	setTemp(0);
		//	new_mode = kMideaACDry;
		//	break;
	default:
		Mode = kMideaACAuto;
	}
}

unsigned char IRMideaAC::CodetoTemperature(unsigned char code)
{
	switch(code)
	{
	case 0:
		return 17; 
		break;
	case 1:
		return 18; 
		break;
	case 3:
		return 19; 
		break;
	case 2:
		return 20; 
		break;
	case 6:
		return 21; 
		break;
	case 7:
		return 22; 
		break;
	case 5:
		return 23; 
		break;
	case 4:
		return 24; 
		break;
	case 12:
		return 25; 
		break;
	case 13:
		return 26; 
		break;
	case 9:
		return 27; 
		break;
	case 8:
		return 28; 
		break;
	case 10:
		return 29; 
		break;
	case 11:
		return 30; 
		break;
	case 14:
		return 0; 
		break;
	}
}

unsigned char IRMideaAC::TemperatureToCode(unsigned char Temp)
{
	switch(Temp)
	{
	case 17:
		return 0; 
		break;
	case 18:
		return 1; 
		break;
	case 19:
		return 3; 
		break;
	case 20:
		return 2; 
		break;
	case 21:
		return 6; 
		break;
	case 22:
		return 7; 
		break;
	case 23:
		return 5; 
		break;
	case 24:
		return 4; 
		break;
	case 25:
		return 12; 
		break;
	case 26:
		return 13; 
		break;
	case 27:
		return 9; 
		break;
	case 28:
		return 8; 
		break;
	case 29:
		return 10; 
		break;
	case 30:
		return 11; 
		break;
	case 0:
		return 14; 
		break;
	}
}


void IRMideaAC::calc() {




	if (Power)
	{
		remote_state[2] = (remote_state[2] & 0x1F) + (FanSpeed<<5);
		remote_state[4] = (remote_state[4] & 0xF3) + (Mode<<2);

		unsigned char tempCode = TemperatureToCode(Temp);
		remote_state[4] = (remote_state[4] & 0xF) + (tempCode<<4);
	}
	else
	{
		remote_state[2] = 0x7B;
		remote_state[4] = 0xE0;
	}


	remote_state[1] = (unsigned char)(~(remote_state[0]));
	remote_state[3] = (unsigned char)(~(remote_state[2]));
	remote_state[5] = (unsigned char)(~(remote_state[4]));
}



bool IRMideaAC::checksum() {

}

//// Convert the internal state into a human readable string.
//#ifdef ARDUINO
//String IRMideaAC::toString() {
//  String result = "";
//#else
//std::string IRMideaAC::toString() {
//  std::string result = "";
//#endif  // ARDUINO
//  result += "Power: ";
//  if (getPower())
//    result += "On";
//  else
//    result += "Off";
//  result += ", Mode: " + uint64ToString(getMode());
//  switch (getMode()) {
//    case kMideaACAuto:
//      result += " (AUTO)";
//      break;
//    case kMideaACCool:
//      result += " (COOL)";
//      break;
//    case kMideaACHeat:
//      result += " (HEAT)";
//      break;
//    case kMideaACDry:
//      result += " (DRY)";
//      break;
//    case kMideaACFan:
//      result += " (FAN)";
//      break;
//    default:
//      result += " (UNKNOWN)";
//  }
//  result += ", Temp: " + uint64ToString(getTemp(true)) + "C/" +
//            uint64ToString(getTemp(false)) + "F";
//  result += ", Fan: " + uint64ToString(getFan());
//  switch (getFan()) {
//    case kMideaACFanAuto:
//      result += " (AUTO)";
//      break;
//    case kMideaACFanLow:
//      result += " (LOW)";
//      break;
//    case kMideaACFanMed:
//      result += " (MED)";
//      break;
//    case kMideaACFanHigh:
//      result += " (HI)";
//      break;
//  }
//  result += ", Sleep: ";
//  if (getSleep())
//    result += "On";
//  else
//    result += "Off";
//  return result;
//}
//
//#if DECODE_MIDEA
//// Decode the supplied Midea message.
////
//// Args:
////   results: Ptr to the data to decode and where to store the decode result.
////   nbits:   The number of data bits to expect. Typically kMideaBits.
////   strict:  Flag indicating if we should perform strict matching.
//// Returns:
////   boolean: True if it can decode it, false if it can't.
////
//// Status: Alpha / Needs testing against a real device.
////
//bool IRrecv::decodeMidea(decode_results *results, uint16_t nbits, bool strict) {
//  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
//    return false;
//
//  uint8_t min_nr_of_messages = 1;
//  if (strict) {
//    if (nbits != kMideaBits) return false;  // Not strictly a MIDEA message.
//    min_nr_of_messages = 2;
//  }
//
//  // The protocol sends the data normal + inverted, alternating on
//  // each byte. Hence twice the number of expected data bits.
//  if (results->rawlen <
//      min_nr_of_messages * (2 * nbits + kHeader + kFooter) - 1)
//    return false;  // Can't possibly be a valid MIDEA message.
//
//  uint64_t data = 0;
//  uint64_t inverted = 0;
//  uint16_t offset = kStartOffset;
//
//  if (nbits > sizeof(data) * 8)
//    return false;  // We can't possibly capture a Midea packet that big.
//
//  for (uint8_t i = 0; i < min_nr_of_messages; i++) {
//    // Header
//    if (!matchMark(results->rawbuf[offset], kMideaHdrMark)) return false;
//    // Calculate how long the common tick time is based on the header mark.
//    uint32_t m_tick = results->rawbuf[offset++] * kRawTick / kMideaHdrMarkTicks;
//    if (!matchSpace(results->rawbuf[offset], kMideaHdrSpace)) return false;
//    // Calculate how long the common tick time is based on the header space.
//    uint32_t s_tick =
//        results->rawbuf[offset++] * kRawTick / kMideaHdrSpaceTicks;
//
//    // Data (Normal)
//    match_result_t data_result = matchData(
//        &(results->rawbuf[offset]), nbits, kMideaBitMarkTicks * m_tick,
//        kMideaOneSpaceTicks * s_tick, kMideaBitMarkTicks * m_tick,
//        kMideaZeroSpaceTicks * s_tick, kMideaTolerance);
//    if (data_result.success == false) return false;
//    offset += data_result.used;
//    if (i % 2 == 0)
//      data = data_result.data;
//    else
//      inverted = data_result.data;
//
//    // Footer
//    if (!matchMark(results->rawbuf[offset++], kMideaBitMarkTicks * m_tick,
//                   kMideaTolerance))
//      return false;
//    if (offset < results->rawlen &&
//        !matchAtLeast(results->rawbuf[offset++], kMideaMinGapTicks * s_tick,
//                      kMideaTolerance))
//      return false;
//  }
//
//  // Compliance
//  if (strict) {
//    // Protocol requires a second message with all the data bits inverted.
//    // We should have checked we got a second message in the previous loop.
//    // Just need to check it's value is an inverted copy of the first message.
//    uint64_t mask = (1ULL << kMideaBits) - 1;
//    if ((data & mask) != ((inverted ^ mask) & mask)) return false;
//    if (!IRMideaAC::validChecksum(data)) return false;
//  }
//
//  // Success
//  results->decode_type = MIDEA;
//  results->bits = nbits;
//  results->value = data;
//  results->address = 0;
//  results->command = 0;
//  return true;
//}
//#endif  // DECODE_MIDEA
