from selfdrive.config import Conversions as CV
from selfdrive.car.honda.values import HONDA_BOSCH
from common.params import Params

# CAN bus layout with relay
# 0 = ACC-CAN - radar side
# 1 = F-CAN B - powertrain
# 2 = ACC-CAN - camera side
# 3 = F-CAN A - OBDII port

def get_pt_bus(car_fingerprint):
  return 1 if car_fingerprint in HONDA_BOSCH else 2 #Clarity: We write to CAN2 to control the car.

def get_lkas_cmd_bus(car_fingerprint, radar_disabled=False):
  if radar_disabled:
    # when radar is disabled, steering commands are sent directly to powertrain bus
    return get_pt_bus(car_fingerprint)
  # normally steering commands are sent to radar, which forwards them to powertrain bus
  return 0


def create_brake_command(packer, apply_brake, pcm_override, pcm_cancel_cmd, fcw, idx, car_fingerprint, stock_brake):
  # TODO: do we loose pressure if we keep pump off for long?
  commands = [] #Clarity
  pump_on = apply_brake > 0 #Clarity: The brake pump algo causes bad braking performance, so we just leave the pump on if the brakes are being called. -wirelessnet2
  brakelights = apply_brake > 0
  brake_rq = apply_brake > 0
  pcm_fault_cmd = False

  #Clarity
  # This a bit of a hack but clarity brake msg flows into the last byte so
  # rather than change the fix() function just set accordingly here. #////I never really understood this. I did not write this comment. The fix() function is old and has been removed. -wirelessnet2
  apply_brake >>= 1
  if apply_brake & 1:
    idx += 0x8

  values = {
    "COMPUTER_BRAKE": apply_brake,
    "BRAKE_PUMP_REQUEST": pump_on,
    "CRUISE_OVERRIDE": pcm_override,
    "CRUISE_FAULT_CMD": pcm_fault_cmd,
    "CRUISE_CANCEL_CMD": pcm_cancel_cmd,
    "COMPUTER_BRAKE_REQUEST": brake_rq,
    "SET_ME_1": 1,
    "BRAKE_LIGHTS": brakelights,
    "CHIME": 1 if fcw else 0,  #Clarity: This calls on stock_brake[] and causes a DANGEROUS software crash during fcw. Chime = 1 is beeping, Chime = 2 is constant tone. -wirelessnet2
    "FCW": fcw << 1,  # TODO: Why are there two bits for fcw?
    "AEB_REQ_1": 0,
    "AEB_REQ_2": 0,
    "AEB_STATUS": 0,
  }
  bus = get_pt_bus(car_fingerprint)
  #return packer.make_can_msg("BRAKE_COMMAND", bus, values, idx)
  commands.append(packer.make_can_msg("BRAKE_COMMAND", bus, values, idx))
  return commands


def create_acc_commands(packer, enabled, accel, gas, idx, stopping, starting, car_fingerprint):
  commands = []
  bus = get_pt_bus(car_fingerprint)

  control_on = 5 if enabled else 0
  # no gas = -30000
  gas_command = gas if enabled and gas > 0 else -30000
  accel_command = accel if enabled else 0
  braking = 1 if enabled and accel < 0 else 0
  standstill = 1 if enabled and stopping else 0
  standstill_release = 1 if enabled and starting else 0

  acc_control_values = {
    # setting CONTROL_ON causes car to set POWERTRAIN_DATA->ACC_STATUS = 1
    "CONTROL_ON": control_on,
    "GAS_COMMAND": gas_command, # used for gas
    "ACCEL_COMMAND": accel_command, # used for brakes
    "BRAKE_LIGHTS": braking,
    "BRAKE_REQUEST": braking,
    "STANDSTILL": standstill,
    "STANDSTILL_RELEASE": standstill_release,
  }
  commands.append(packer.make_can_msg("ACC_CONTROL", bus, acc_control_values, idx))

  acc_control_on_values = {
    "SET_TO_3": 0x03,
    "CONTROL_ON": enabled,
    "SET_TO_FF": 0xff,
    "SET_TO_75": 0x75,
    "SET_TO_30": 0x30,
  }
  commands.append(packer.make_can_msg("ACC_CONTROL_ON", bus, acc_control_on_values, idx))

  return commands

def create_steering_control(packer, apply_steer, lkas_active, car_fingerprint, idx, radar_disabled):
  values = {
    "STEER_TORQUE": apply_steer if lkas_active else 0,
    "STEER_TORQUE_REQUEST": lkas_active,
  }
  bus = get_lkas_cmd_bus(car_fingerprint, radar_disabled)
  return packer.make_can_msg("STEERING_CONTROL", bus, values, idx)


def create_bosch_supplemental_1(packer, car_fingerprint, idx):
  # non-active params
  values = {
    "SET_ME_X04": 0x04,
    "SET_ME_X80": 0x80,
    "SET_ME_X10": 0x10,
  }
  bus = get_lkas_cmd_bus(car_fingerprint)
  return packer.make_can_msg("BOSCH_SUPPLEMENTAL_1", bus, values, idx)


def create_ui_commands(packer, pcm_speed, hud, car_fingerprint, is_metric, idx, openpilot_longitudinal_control, stock_hud):
  commands = []
  bus_pt = get_pt_bus(car_fingerprint)
  radar_disabled = car_fingerprint in HONDA_BOSCH and openpilot_longitudinal_control
  bus_lkas = get_lkas_cmd_bus(car_fingerprint, radar_disabled)

  is_eon_metric = Params().get("IsMetric", encoding='utf8') == "1"
  if is_eon_metric:
    speed_units = 2
  else:
    speed_units = 3

  if openpilot_longitudinal_control:
    if car_fingerprint in HONDA_BOSCH:
      acc_hud_values = {
        'CRUISE_SPEED': hud.v_cruise,
        'ENABLE_MINI_CAR': 1,
        'SET_TO_1': 1,
        'HUD_LEAD': hud.car,
        'HUD_DISTANCE': 3,
        'ACC_ON': hud.car != 0,
        'SET_TO_X1': 1,
        'IMPERIAL_UNIT': int(not is_metric),
      }
    else:
      acc_hud_values = {
      'PCM_SPEED': pcm_speed * CV.MS_TO_KPH,
      'PCM_GAS': hud.pcm_accel,
      'CRUISE_SPEED': hud.v_cruise,
      'ENABLE_MINI_CAR': 1,
      'HUD_LEAD': hud.car,
      'HUD_DISTANCE_3': 1,  
      'HUD_DISTANCE': hud.dist_lines,
      'IMPERIAL_UNIT': speed_units,
      'SET_ME_X01_2': 1,
      'SET_ME_X01': 1,
      "FCM_OFF": 0, #CLarity: This call on stock_hud[] and causes a crash. -wirelessnet2
      "FCM_OFF_2": 0, #CLarity: This call on stock_hud[] and causes a crash. -wirelessnet2
      "FCM_PROBLEM": 0, #CLarity: This call on stock_hud[] and causes a crash. -wirelessnet2
      "ICONS": 0, #CLarity: This call on stock_hud[] and causes a crash. -wirelessnet2
    }
    commands.append(packer.make_can_msg("ACC_HUD", bus_pt, acc_hud_values, idx))

  lkas_hud_values = {
    'SET_ME_X41': 0x41,
    'SET_ME_X48': 0x48,
    'STEERING_REQUIRED': hud.steer_required,
    'SOLID_LANES': hud.lanes,
    'DASHED_LANES': hud.dashed_lanes,
    'BEEP': 0,
  }
  commands.append(packer.make_can_msg('LKAS_HUD', bus_lkas, lkas_hud_values, idx))

  if radar_disabled and car_fingerprint in HONDA_BOSCH:
    radar_hud_values = {
      'SET_TO_1' : 0x01,
    }
    commands.append(packer.make_can_msg('RADAR_HUD', bus_pt, radar_hud_values, idx))

  return commands

#Clarity: Since we don't have a factory ADAS Camera to drive the Radar, we have to create the messages ourselves. -wirelessnet2
def create_radar_commands(packer, vEgoRawKph, idx):
  """Creates an iterable of CAN messages for the radar system."""
  commands = []
  radar_bus = 1

  msg300 = {
    'SET_ME_XF9': 0xF9,
    'VEHICLE_SPEED': vEgoRawKph,
    'SET_ME_X8A': 0x8A,
    'SET_ME_XD0': 0xD0,
    'SALTED_WITH_IDX': 0x20 if idx == 0 or idx == 3 else 0x00,
  }
  msg301 = {
    'SET_ME_X5D': 0x5D, #This is 1/3 of the Vehicle State MSG for Clarity. -wirelessnet2
    'SET_ME_X02': 0x02, #This is 1/3 of the Vehicle State MSG for Clarity. -wirelessnet2
    'SET_ME_X5F': 0x5F, #This is 1/3 of the Vehicle State MSG for Clarity. -wirelessnet2
  }
  commands.append(packer.make_can_msg('VEHICLE_STATE', radar_bus, msg300, idx))
  commands.append(packer.make_can_msg('VEHICLE_STATE2', radar_bus, msg301, idx))
  #THANKS ENERGEE UR THE BEST -wirelessnet2
  return commands

def spam_buttons_command(packer, button_val, idx, car_fingerprint):
  values = {
    'CRUISE_BUTTONS': button_val,
    'CRUISE_SETTING': 0,
  }
  bus = get_pt_bus(car_fingerprint)
  return packer.make_can_msg("SCM_BUTTONS", bus, values, idx)
