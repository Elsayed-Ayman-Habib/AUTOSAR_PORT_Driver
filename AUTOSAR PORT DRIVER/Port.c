 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port.c
 *
 * Description: Source file for TM4C123GH6PM Microcontroller - Port Driver
 *
 * Author: Elsayed Ayman Elsayed Ali Habib
 ******************************************************************************/

#include "Port.h"
#include "Port_Regs.h"
#include "tm4c123gh6pm_registers.h"

#if (PORT_DEV_ERROR_DETECT == STD_ON)

#include "Det.h"
/* AUTOSAR Version checking between Det and Port Modules */
#if ((DET_AR_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
 || (DET_AR_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
 || (DET_AR_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Det.h does not match the expected version"
#endif

#endif

STATIC const Port_ConfigChannel * Port_Configuration = NULL_PTR;
STATIC uint8 Port_Status = PORT_NOT_INITIALIZED;

/************************************************************************************
* Service Name: Port_Init
* Service ID[hex]: 0x00
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): ConfigPtr - Pointer to post-build configuration data
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Function to Initialize the Port module.
************************************************************************************/
void Port_Init(const Port_ConfigType * ConfigPtr)
{
  #if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* check if the input configuration pointer is not a NULL_PTR */
  if (ConfigPtr == NULL_PTR)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_INIT_SID,
      PORT_E_PARAM_CONFIG);
    }
    else
    #endif
    {
      /*
      * Set the module state to initialized and point to the PB configuration structure using a global pointer.
      * This global pointer is global to be used by other functions to read the PB configuration structures
      */
      Port_Status       = PORT_INITIALIZED;
      Port_Configuration = ConfigPtr->Channels; /* address of the first Channels structure --> Channels[0] */
    }
    for(uint8 i = 0; i < PORT_CONFIGURED_CHANNLES; i++){

      volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
      volatile uint32 delay = 0;
      /*
      * Switch case to determine the Base address of the Port from the attribute port_num in the struct ConfigPtr
      */
      switch(Port_Configuration[i].port_num)
      {
        case  0: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
        break;
        case  1: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
        break;
        case  2: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
        break;
        case  3: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
        break;
        case  4: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
        break;
        case  5: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
        break;
      }

      /* Enable clock for PORT and allow time for clock to start*/
      SYSCTL_REGCGC2_REG |= (1<<Port_Configuration[i].port_num);
      delay = SYSCTL_REGCGC2_REG;

      if( ((Port_Configuration[i].port_num == 3) && (Port_Configuration[i].pin_num == 7)) || ((Port_Configuration[i].port_num == 5) && (Port_Configuration[i].pin_num == 0)) ) /* PD7 or PF0 */
      {
        *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = GPIO_COMMIT_UNLOCK_NUMBER;                     /* Unlock the GPIOCR register */
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_Configuration[i].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
      }
      else if( (Port_Configuration[i].port_num == 2) && (Port_Configuration[i].pin_num <= 3) ) /* PC0 to PC3 */
      {
        continue;
        /* Do Nothing ...  this is the JTAG pins */
      }
      else
      {
        /* Do Nothing ... No need to unlock the commit register for this pin */
      }

      if(Port_Configuration[i].mode == PORT_PIN_MODE_ADC)
      {
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Configuration[i].pin_num);      /* SET the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[i].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[i].pin_num);             /* Disable Digitale Enable for this pin by clear the corresponding bit in GPIODEN register */
      }
      else if(Port_Configuration[i].mode == PORT_PIN_MODE_DIO){
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Configuration[i].pin_num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[i].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
        *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_Configuration[i].pin_num * 4));     /* Clear the PMCx bits for this pin */
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[i].pin_num);         /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
      }
      else{
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[i].pin_num);              /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
        *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= ((0x0000000F & Port_Configuration[i].mode) << (Port_Configuration[i].pin_num * 4));
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[i].pin_num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
      }

      if(Port_Configuration[i].direction == PORT_PIN_OUT)
      {
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[i].pin_num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */

        if(Port_Configuration[i].init_Val == STD_HIGH)
        {
          SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_Configuration[i].pin_num);          /* Set the corresponding bit in the GPIODATA register to provide initial value 1 */
        }
        else
        {
          CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_Configuration[i].pin_num);        /* Clear the corresponding bit in the GPIODATA register to provide initial value 0 */
        }
      }
      else if(Port_Configuration[i].direction == PORT_PIN_IN)
      {
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[i].pin_num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */

        if(Port_Configuration[i].resistor_state == PULL_UP)
        {
          SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_Configuration[i].pin_num);       /* Set the corresponding bit in the GPIOPUR register to enable the internal pull up pin */
        }
        else if(Port_Configuration[i].resistor_state == PULL_DOWN)
        {
          SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_Configuration[i].pin_num);     /* Set the corresponding bit in the GPIOPDR register to enable the internal pull down pin */
        }
        else
        {
          CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_Configuration[i].pin_num);     /* Clear the corresponding bit in the GPIOPUR register to disable the internal pull up pin */
          CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_Configuration[i].pin_num);   /* Clear the corresponding bit in the GPIOPDR register to disable the internal pull down pin */
        }
      }
      else
      {
        /* Do Nothing */
      }
  }
    
}

/************************************************************************************
* Service Name: Port_SetPinDirection
* Service ID[hex]: 0x01
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): Pin - Port Pin ID number.
*                  Direction - Port Pin direction.
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Sets the Port Pin direction.
************************************************************************************/
#if (PORT_SET_PIN_DIRECTION_API == STD_ON)
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction){
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* Check if the input Pin is not suitable to the range of pins */
  if ((Pin < 0) || (Pin > 42))
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
                    PORT_E_PARAM_PIN);
  }
  /* Check if the Port module isnt initialized */
  else if(Port_Status == PORT_NOT_INITIALIZED){
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
                    PORT_E_UNINIT);
  }
  /* check if Port Pin not configured as changeable */
  else if (Port_Configuration[Pin].Dir_changable == STD_OFF){
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID, PORT_E_DIRECTION_UNCHANGEABLE);
  }
  else
#endif
  {
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    volatile uint32 delay = 0;
    /*
    * Switch case to determine the Base address of the Port from the attribute port_num in the struct ConfigPtr
    */
    switch(Port_Configuration[Pin].port_num)
    {
    case  0: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
    break;
    case  1: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
    break;
    case  2: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
    break;
    case  3: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
    break;
    case  4: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
    break;
    case  5: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
    break;
    }


    if( ((Port_Configuration[Pin].port_num == 3) && (Port_Configuration[Pin].pin_num == 7)) || ((Port_Configuration[Pin].port_num == 5) && (Port_Configuration[Pin].pin_num == 0)) ) /* PD7 or PF0 */
    {
      *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = GPIO_COMMIT_UNLOCK_NUMBER;                     /* Unlock the GPIOCR register */
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_Configuration[Pin].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
    }
    else if( (Port_Configuration[Pin].port_num == 2) && (Port_Configuration[Pin].pin_num <= 3) ) /* PC0 to PC3 */
    {
      /* Do Nothing ...  this is the JTAG pins */
      return;
    }
    else
    {
      /* Do Nothing ... No need to unlock the commit register for this pin */
    }
    if(Direction == PORT_PIN_OUT)
    {
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[Pin].pin_num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
    }
    else if(Direction == PORT_PIN_IN)
    {
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[Pin].pin_num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
    }
    else
    {
      /* Do Nothing */
    }
  }
}
#endif
/************************************************************************************
* Service Name: Port_RefreshPortDirection
* Service ID[hex]: 0x02
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Refreshes port direction.
************************************************************************************/
void Port_RefreshPortDirection( void ){
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* Check if the Port module isnt initialized */
  if(Port_Status == PORT_NOT_INITIALIZED){
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_REFRESH_PORT_DIR_SID,
                    PORT_E_UNINIT);
  }
#endif
  for(uint8 i = 0; i < PORT_CONFIGURED_CHANNLES; i++){
    
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    volatile uint32 delay = 0;
    
    /*PORT061: The function Port_RefreshPortDirection shall exclude those port pins 
    from refreshing that are configured as ‘pin direction changeable during runtime‘.*/
    if(Port_Configuration[i].Dir_changable != STD_ON){/* Check if Pin direction is changable during runtime */
      continue;
    }
    else{
      /*
      * Switch case to determine the Base address of the Port from the attribute port_num in the struct ConfigPtr
      */
      switch(Port_Configuration[i].port_num)
      {
      case  0: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
      break;
      case  1: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
      break;
      case  2: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
      break;
      case  3: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
      break;
      case  4: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
      break;
      case  5: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
      break;
      }
      if( ((Port_Configuration[i].port_num == 3) && (Port_Configuration[i].pin_num == 7)) || ((Port_Configuration[i].port_num == 5) && (Port_Configuration[i].pin_num == 0)) ) /* PD7 or PF0 */
      {
        *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = GPIO_COMMIT_UNLOCK_NUMBER;                     /* Unlock the GPIOCR register */
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_Configuration[i].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
      }
      else if( (Port_Configuration[i].port_num == 2) && (Port_Configuration[i].pin_num <= 3) ) /* PC0 to PC3 */
      {
        continue;
        /* Do Nothing ...  this is the JTAG pins */
      }
      else
      {
        /* Do Nothing ... No need to unlock the commit register for this pin */
      }
      if(Port_Configuration[i].direction == PORT_PIN_OUT)
      {
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[i].pin_num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
      }
      else if(Port_Configuration[i].direction == PORT_PIN_IN)
      {
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Configuration[i].pin_num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */      
      }
      else
      {
        /* Do Nothing */
      }
    }
  }
}
/************************************************************************************
* Service Name: Port_GetVersionInfo
* Service ID[hex]: 0x03
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): VersionInfo - Pointer to where to store the version information of this module.
* Return value: None
* Description: Function to get the version information of this module.
************************************************************************************/
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* Check if input pointer is not Null pointer */
  if(NULL_PTR == versioninfo)
  {
    /* Report to DET  */
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID,
                    PORT_GET_VERSION_INFO_SID, PORT_E_PARAM_POINTER);
  }
  else
  #endif /* (PORT_DEV_ERROR_DETECT == STD_ON) */
	{
		/* Copy the vendor Id */
		versioninfo->vendorID = (uint16)PORT_VENDOR_ID;
		/* Copy the module Id */
		versioninfo->moduleID = (uint16)PORT_MODULE_ID;
		/* Copy Software Major Version */
		versioninfo->sw_major_version = (uint8)PORT_SW_MAJOR_VERSION;
		/* Copy Software Minor Version */
		versioninfo->sw_minor_version = (uint8)PORT_SW_MINOR_VERSION;
		/* Copy Software Patch Version */
		versioninfo->sw_patch_version = (uint8)PORT_SW_PATCH_VERSION;
	}
}
#endif

/************************************************************************************
* Service Name: Port_SetPinMode
* Service ID[hex]: 0x04
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): Pin - Port Pin ID number.
*                  Mode - New Port Pin mode to be set on port pin.
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Sets the port pin mode.
************************************************************************************/
#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode){
  #if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* Check if the input Pin is not suitable to the range of pins */
  if ((Pin < 0) || (Pin > 42))
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
      PORT_E_PARAM_PIN);
  }
  else
  {

  }
  /* Check if the Port module isnt initialized */
  if(Port_Status == PORT_NOT_INITIALIZED){
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
      PORT_E_UNINIT);
  }
  else
  {

  }
  if((Mode < 0) || (Mode > 10))
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
      PORT_E_PARAM_INVALID_MODE);
  }
  else
  {

  }
  if(Port_Configuration[Pin].Mode_changable != STD_ON)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
      PORT_E_MODE_UNCHANGEABLE);
  }
  else
  {

  }

  #endif
  {
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    volatile uint32 delay = 0;
    /*
    * Switch case to determine the Base address of the Port from the attribute port_num in the struct ConfigPtr
    */
    switch(Port_Configuration[Pin].port_num)
    {
      case  0: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
      break;
      case  1: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
      break;
      case  2: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
      break;
      case  3: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
      break;
      case  4: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
      break;
      case  5: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
      break;
    }



    if( ((Port_Configuration[Pin].port_num == 3) && (Port_Configuration[Pin].pin_num == 7)) || ((Port_Configuration[Pin].port_num == 5) && (Port_Configuration[Pin].pin_num == 0)) ) /* PD7 or PF0 */
    {
      *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = GPIO_COMMIT_UNLOCK_NUMBER;                     /* Unlock the GPIOCR register */
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_Configuration[Pin].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
    }
    else if( (Port_Configuration[Pin].port_num == 2) && (Port_Configuration[Pin].pin_num <= 3) ) /* PC0 to PC3 */
    {
      return;
      /* Do Nothing ...  this is the JTAG pins */
    }
    else
    {
      /* Do Nothing ... No need to unlock the commit register for this pin */
    }
    if(Mode == PORT_PIN_MODE_ADC)
    {
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Configuration[Pin].pin_num);      /* SET the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[Pin].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[Pin].pin_num);             /* Disable Digitale Enable for this pin by clear the corresponding bit in GPIODEN register */
    }
    else if(Port_Configuration[Pin].mode == PORT_PIN_MODE_DIO){
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Configuration[Pin].pin_num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[Pin].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
      *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_Configuration[Pin].pin_num * 4));     /* Clear the PMCx bits for this pin */
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[Pin].pin_num);         /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
    else{
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Configuration[Pin].pin_num);              /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
      *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= ((0x0000000F & Mode) << (Port_Configuration[Pin].pin_num * 4));
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Configuration[Pin].pin_num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
  }
}
#endif