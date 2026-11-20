    

#include "meins.h"
#include "main.h"
#include "stdint.h"
#include "deca_device_api.h"








//variablen:
static uint8_t timestamp[5];
static uint8_t rx_timestamp[5];
static uint8_t rx_timestamp2[5];
static uint64_t delay = 0;
static double distance = 0;
static uint8_t rx_buffer[128];



#define TS_MASK 0xFFFFFFFFFFULL //maske um die 40 bit timestamp overflow zu behandeln.
#define uDelay 1000 //1ms wartezeit bis timeout in us

void send_ranging(void){
  
  uint8_t tx_msg[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0x43, 0x02, 0, 0 };
   
  
  //dwt_forcetrxoff();
  //send the message
  dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
  dwt_writetxdata(sizeof(tx_msg), tx_msg, 0);
  dwt_writetxfctrl(sizeof(tx_msg), 0, 1);
  dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
  //warte bis Übertragung abgeschlossen ist
  uint32_t start = LL_TIM_GetCounter(TIM1);
  while (!(dwt_readsysstatuslo() & DWT_INT_TXFRS_BIT_MASK)) 
  {
    if(((LL_TIM_GetCounter(TIM1)-start) & UINT16_MAX) > uDelay)
    {
      // Timeout occurred
      return;
    }
  }
  //speichere die Zeit des Sendens in timestamp und lösche das Flag
  dwt_readtxtimestamp(timestamp);
  dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
  //warte auf die Antwort und falls paket gut speichere die zeit den inhalt(inhalt hier weglassen?) und lösche das Flag
  waitforsysstatus(NULL, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR), 0);
  if(dwt_readsysstatuslo() & DWT_INT_RXFCG_BIT_MASK)
  {
    dwt_readrxdata(rx_buffer, dwt_getframelength(0), 0);
    dwt_readrxtimestamp(rx_timestamp,DWT_COMPAT_NONE);
    dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
  }
  else //fehlerhaftes Paket empfangen
  {
    dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR); 
    return;
  }
  //in dieser message sollen jetzt auch die timestamps mitgeschickt werden, um das ganze 2 way zu machen kann man hier auch mit delayed send weitere timestamps mitsenden
  uint8_t tx_msg2[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',0x43, 0x02, 0,0,0,0,0,0,0,0,0,0, 0, 0 }; 
  for(int i = 0; i < 5; i++) //die timestamps in das zu sendende Paket schreiben
  {
    tx_msg2[12 + i] = timestamp[i]; //10 stellen nach DECAWAVE ersten 5 die sendezeit des ersten und danach die empfangszeit des antwortpakets
    tx_msg2[17 + i] = rx_timestamp[i]; 
  }
  //sende die 2te message mit den timestamps
  dwt_writetxdata(sizeof(tx_msg2), tx_msg2, 0);
  dwt_writetxfctrl(sizeof(tx_msg2), 0, 1);
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  
}

void receive_ranging(void){
  uint8_t initiator_timestamp[5];
  uint8_t initiator_rx_timestamp[5];
  
  
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  waitforsysstatus(NULL, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR), 0);
  if(dwt_readsysstatuslo() & DWT_INT_RXFCG_BIT_MASK)
  {
    dwt_readrxdata(rx_buffer, dwt_getframelength(0), 0);
    dwt_readrxtimestamp(rx_timestamp,DWT_COMPAT_NONE);
    dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
  }
  else //fehlerhaftes Paket empfangen
  {
    dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR); 
    return;
  }


  uint8_t tx_msg[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0x43, 0x02, 0, 0 }; //nachricht aus beispiel ist es hier wichtig sich an die standards zu halten?
  dwt_forcetrxoff();
  dwt_writetxdata(sizeof(tx_msg), tx_msg, 0);
  dwt_writetxfctrl(sizeof(tx_msg), 0, 1);
  dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
  uint32_t start = LL_TIM_GetCounter(TIM1);
  while (!(dwt_readsysstatuslo() & DWT_INT_TXFRS_BIT_MASK))//warte bis Übertragung abgeschlossen ist
  {
    if(((LL_TIM_GetCounter(TIM1)-start) & UINT16_MAX) > uDelay)
    {
      // Timeout occurred
      return;
    }
  }
  dwt_readtxtimestamp(timestamp);
  dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);


  waitforsysstatus(NULL, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR), 0); //wait for the response
  if(dwt_readsysstatuslo() & DWT_INT_RXFCG_BIT_MASK) //read response if ok
  {
    dwt_readrxdata(rx_buffer, dwt_getframelength(0), 0);
    dwt_readrxtimestamp(rx_timestamp2,DWT_COMPAT_NONE);
    dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
    
    for(int i = 0; i < 5; i++) //die anderen timestamps aus dem empfangenen Paket auslesen
    {
      initiator_timestamp[i] = rx_buffer[12 + i];
      initiator_rx_timestamp[i] = rx_buffer[17 + i];
    }
  }
  else //fehlerhaftes Paket empfangen
  {
    dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR); 
    return;
  }
  uint64_t time_of_flight;
  //zeit zwichen empfangen und senden der nachricht
  delay = timestamp_to_64bit(timestamp) - timestamp_to_64bit(rx_timestamp); 
  //gesamte zeit zwichen ersten sendung und dem ankommen der antwort minus dem delay im DW selber also nur die flugzeit
  time_of_flight = timestamp_to_64bit(initiator_rx_timestamp) - timestamp_to_64bit(initiator_timestamp) - delay; 
  //flugzeit*lightspeed /2 *conversion from UWB time units to seconds also sollte hier die Distanz in Metern rauskommen
  distance = (double) time_of_flight * 149896229 * (1.0 / (499.2e6 * 128.0)); 
}

uint64_t timestamp_to_64bit(uint8_t *timestamp) //evtl anders herum wenn die bytes in der falschen reihenfolge sind Little-Endian vs Big-Endian
{
  uint64_t ts = 0;
  for (int i = 0; i < 5; i++)
  {
    ts <<= 8;
    ts |= timestamp[i];
  }
  return ts & TS_MASK; //maske um die 40 bit timestamp overflow zu behandeln.
}

// uint64_t timestamp_to_64bit(uint8_t *timestamp) //evtl anders herum wenn die bytes in der falschen reihenfolge sind Little-Endian vs Big-Endian
// {
//   uint64_t ts = 0;
//   for (int i = 5; i > 0; i--)
//   {
//     ts <<= 8;
//     ts |= timestamp[i];
//   }
//   return ts & TS_MASK; //maske um die 40 bit timestamp overflow zu behandeln.
// }

void waitforsysstatus(uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask)
{
    uint32_t lo_result_tmp = 0;
    uint32_t hi_result_tmp = 0;
    uint32_t start = LL_TIM_GetCounter(TIM1);

    // If a mask has been passed into the function for the system status register (lower 32-bits)
    if (lo_mask)
    {
        while (!((lo_result_tmp = dwt_readsysstatuslo()) & (lo_mask)))
        {
            // If a mask value is set for the system status register (higher 32-bits)
            if (hi_mask)
            {
                // If mask value for the system status register (higher 32-bits) is found
                if ((hi_result_tmp = dwt_readsysstatushi()) & hi_mask)
                {
                    break;
                }
            }
            if(((LL_TIM_GetCounter(TIM1)-start) & UINT16_MAX) > uDelay)
                {
                  // Timeout occurred
                  return;
                }
        }
    }
    // if only a mask value for the system status register (higher 32-bits) is set
    else if (hi_mask)
    {
        while (!((hi_result_tmp = dwt_readsysstatushi()) & (hi_mask))) {
          if(((LL_TIM_GetCounter(TIM1)-start) & UINT16_MAX) > uDelay)
                {
                  // Timeout occurred
                  return;
                }
         };
    }

    if (lo_result != NULL)
    {
        *lo_result = lo_result_tmp;
    }

    if (hi_result != NULL)
    {
        *hi_result = hi_result_tmp;
    }
}


