//
//  CCServerManager.swift
//  chickencoop
//
//  Created by Vincent Moscaritolo on 1/1/22.
//

import Foundation
import SwiftRadix
import CoreLocation
import UIKit

//
//extension CaseIterable where Self: RawRepresentable {
//
//	 static var allValues: [RawValue] {
//		  return allCases.map { $0.rawValue }
//	 }
//}

enum  device_state_t :Int{
	case
		DEVICE_STATE_UNKNOWN = 0,
		DEVICE_STATE_DISCONNECTED,
		DEVICE_STATE_CONNECTED,
		DEVICE_STATE_ERROR,
		DEVICE_STATE_TIMEOUT
 }
 
extension device_state_t: Codable {
	
	enum Key: CodingKey {
		case rawValue
	}
	
	enum CodingError: Error {
		case unknownValue
	}
	
	init(from decoder: Decoder) throws {
		let container = try decoder.container(keyedBy: Key.self)
		let rawValue = try container.decode(Int.self, forKey: .rawValue)
		switch rawValue {
		case 0:
			self = .DEVICE_STATE_UNKNOWN
		case 1:
			self = .DEVICE_STATE_DISCONNECTED
		case 2:
			self = .DEVICE_STATE_CONNECTED
		case 3:
			self = .DEVICE_STATE_ERROR
		case 4:
			self = .DEVICE_STATE_TIMEOUT
	
		default:
			self = .DEVICE_STATE_UNKNOWN
		}
	}
	
	func encode(to encoder: Encoder) throws {
		var container = encoder.container(keyedBy: Key.self)
		switch self {
		case .DEVICE_STATE_UNKNOWN:
			try container.encode(0, forKey: .rawValue)
		case .DEVICE_STATE_DISCONNECTED:
			try container.encode(1, forKey: .rawValue)
		case .DEVICE_STATE_CONNECTED:
			try container.encode(2, forKey: .rawValue)
		case .DEVICE_STATE_ERROR:
			try container.encode(3, forKey: .rawValue)
		case .DEVICE_STATE_TIMEOUT:
			try container.encode(4, forKey: .rawValue)
		}
	}
}


enum  DoorState :Int{
	case
		unknown = 0,
		open,
		opening,
		closed,
		closing
}
 
extension DoorState: Codable {
	
	enum Key: CodingKey {
		case rawValue
	}
	
	enum CodingError: Error {
		case unknownValue
	}
	
	init(from decoder: Decoder) throws {
		let container = try decoder.container(keyedBy: Key.self)
		let rawValue = try container.decode(Int.self, forKey: .rawValue)
		switch rawValue {
		case 0:
			self = .unknown
		case 1:
			self = .open
		case 2:
			self = .opening
		case 3:
			self = .closed
		case 4:
			self = .closing
		default:
			self = .unknown
		}
	}
	
	func encode(to encoder: Encoder) throws {
		var container = encoder.container(keyedBy: Key.self)
		switch self {
		case .unknown:
			try container.encode(0, forKey: .rawValue)
		case .open:
			try container.encode(1, forKey: .rawValue)
		case .opening:
			try container.encode(2, forKey: .rawValue)
		case .closed:
			try container.encode(3, forKey: .rawValue)
		case .closing:
			try container.encode(4, forKey: .rawValue)
		}
	}
	
	func description() -> String {
		var str = "Unknown"
		
		switch self {
		case .open:			str = "Open"
		case .opening:		str = "Opening"
		case .closed:		str = "Closed"
		case .closing: str = "Closing"
			
		default:
			break
		}
		return str
	}
}


class RESTErrorInfo: Codable {
	let code: Int
	let message: String
	let detail: String?
	let cmdDetail: String?
}

class RESTError: Codable {
	let error: RESTErrorInfo
}
 
enum RESTdeviceStatus : Int {
	case unknown = 0
	case disconnected
	case connected
	case error
	case timeout
}


struct RESTStatus: Codable {
	var state: Int
	var cpuTemp: Double?
	var stateString: String?
	var buildtime: String
	var version: String
	let date: String
	let uptime: Int
	let os_sysname : String
	let os_nodename : String
	let os_release : String
	let os_version : String
	let os_machine : String

	enum CodingKeys: String, CodingKey {
		case state   		= "state"
		case cpuTemp  		= "cpuTemp"
		case stateString  	= "stateString"
		case buildtime 		= "buildtime"
		case version 		= "version"
		case date 				= "date"
		case uptime 			= "uptime"
		case os_sysname 		= "os.sysname"
		case os_nodename 	= "os.nodename"
		case os_release 		= "os.release"
		case os_version 		= "os.version"
		case os_machine 		= "os.machine"
	}

}

class RESTDateInfo: Codable {
	let civilSunRise: Double?
	let civilSunSet: Double?
	let sunRise: Double?
	let sunSet: Double?
	let date: String
	let gmtOffset: Int?
	let latitude: Double?
	let longitude: Double?
	let midnight: Double?
	let timeZone: String?
	let uptime: Int
}


struct RESTValueDetails: Codable {
	var display: String
	var time:  Double
	var value: String
	
	enum CodingKeys: String, CodingKey {
		case display = "display"
		case time = "time"
		case value = "value"
	}
}
 
enum RESTschemaTracking : Int {
	case ignoreValue = 0
	case staticValue = 1
	case trackedValue = 2
	case untrackedValue = 3
}
 
enum RESTschemaUnits : Int {
	case INVALID = 0
	case BOOL				// Bool ON/OFF
	case INT			// Int
	case MAH				// mAh milliAmp hours
	case PERMILLE		// (per thousand) sign ‰
	case PERCENT		// (per hundred) sign ‰
	case DEKAWATTHOUR	 // .01kWh
	case WATTS			// W
	case MILLIVOLTS		// mV
	case MILLIAMPS		// mA
	case SECONDS			// sec
	case MINUTES			// mins
	case DEGREES_C		// degC
	case VOLTS			// V
	case HERTZ			// Hz
	case AMPS				// A
	case BINARY			// Binary 8 bits 000001
	case VE_PRODUCT		// VE.PART
	case STRING			// string
	case IGNORE
	case DOOR_STATE
	case ON_OFF
	case UNKNOWN
}

struct RESTSchemaDetails: Codable {
	var name: String
	var suffix:  String
	var tracking: Int
	var units: Int

	enum CodingKeys: String, CodingKey {
		case name = "name"
		case suffix = "suffix"
		case tracking = "tracking"
		case units = "units"
	}
	init() {
		name = String()
		suffix = String()
		tracking = RESTschemaTracking.ignoreValue.rawValue
		units = RESTschemaUnits.UNKNOWN.rawValue
	}
}

struct RESTSchemaList: Codable {
	var schema:  Dictionary<String, RESTSchemaDetails>

	enum CodingKeys: String, CodingKey {
		case schema = "schema"
	}
}


struct RESTProperties: Codable {
	var properties:  Dictionary<String, String>
  }

struct RESTValuesList: Codable {
	var values:  Dictionary<String, RESTValueDetails>
	var ETag: 	Int?
	
	enum CodingKeys: String, CodingKey {
		case values = "values"
		case ETag = "ETag"
	}
  }

struct RESTTimeSpanItem: Codable {
	var time:  		Double
	var durration:  TimeInterval
	var value: 		String
}

struct RESTHistoryItem: Codable {
	var time:  Double
	var value: String
	
	enum CodingKeys: String, CodingKey {
		case time = "time"
		case value = "value"
	}
}

struct RESTHistory: Codable {
	var values:  Array< RESTHistoryItem>
	enum CodingKeys: String, CodingKey {
		case values = "values"
	}
 
	init() {
		values = []
	}
	
	func timeLine() -> Array<RESTTimeSpanItem> {
		
		var timeline:Array<RESTTimeSpanItem> = []
		
		let items = self.values.reversed()
		let now = Date().timeIntervalSince1970
		var lastValue:String = "<no value>"
		var lastTime:Double = 0;
		
		for item in items {
		
			// did we change values
			if (item.value != lastValue){
				
				// if this is the first change - we subtract time from present
				let interval:TimeInterval
					= (lastTime == 0) ? now - item.time  : lastTime - item.time
				
				lastTime = item.time
				lastValue = item.value
				
				timeline.append( RESTTimeSpanItem(time: item.time,
															 durration: interval, value: item.value))
			}
		}
		
		return timeline
	}
	
	
}


struct RESTEventAction: Codable {
	var cmd			 		:String?
	var deviceID			:String?

	enum CodingKeys: String, CodingKey {
		case cmd
		case deviceID
	}
}

struct RESTEventTrigger: Codable {
	var mins 				:Int?
	var timeBase			:Int?
	var event				:String?

	enum CodingKeys: String, CodingKey {
		case mins
		case timeBase
		case event
	}
	
	func triggerDate(_ solarTimes: ServerDateInfo ) ->Date {
		
		var solarTimeFormat: DateFormatter {
			let formatter = DateFormatter()
			formatter.dateFormat = "hh:mm:ss a"
			formatter.timeZone = TimeZone(abbreviation: "UTC")
			return formatter
		}
		
		var date: Date = Date.distantPast
		
		if let sunSet = solarTimes.sunSet,
			let civilSunSet = solarTimes.civilSunSet,
			let civilSunRise = solarTimes.civilSunRise,
			let sunRise = solarTimes.sunRise,
			let midNight = solarTimes.midNight,
			let mins = self.mins {
			
			switch  self.timeBase {
			case 1: // TOD_ABSOLUTE
				date = midNight.addingTimeInterval( Double(mins * 60))
				break
				
			case 2: // TOD_SUNRISE
				date = sunRise.addingTimeInterval( Double(mins * 60))
				break
				
			case 3: // TOD_SUNSET
				date = sunSet.addingTimeInterval( Double(mins * 60))
				break
				
			case 4: // TOD_CIVIL_SUNRISE
				date = civilSunRise.addingTimeInterval( Double(mins * 60))
				break
				
			case 5: // TOD_CIVIL_SUNSET
				date = civilSunSet.addingTimeInterval( Double(mins * 60))
				break
				
			default:
				break
			}
		}
		return date
	}
}


struct RESTEventCreation: Codable {
	var eventID : String?
 	var name		: String
  }

struct RESTEvent: Codable {
	var eventID : String?

	var name		: String
	var action	: 	RESTEventAction
	var trigger	: 	RESTEventTrigger
	
	init() {
		name = String()
		action = RESTEventAction()
		action.cmd =  String()
		action.deviceID =  String()
		trigger = RESTEventTrigger()
		trigger.timeBase = .none
		trigger.mins = 0
		trigger.event = nil
	}
	
	func JSON() throws -> Data? {
		var json : [String: Any] = [:]
		json["name"] = name
		json["action"] = ["cmd" : action.cmd,
							"deviceID" : action.deviceID ]
	 
		if isAppEvent() {
			json["trigger"] = ["event": trigger.event]
		}
		else if isTimedEvent() {
			json["trigger"] = ["mins":   trigger.mins,
									 "timeBase":   trigger.timeBase ]
		}
	 
		let jsonData = try? JSONSerialization.data(withJSONObject: json)
		return jsonData
	}
	
	func isValid() -> Bool {
		return
			self.eventType() != .unknown
			&& deviceIDValue() != .invalid
			&& cmdValue() != .invalid
	}
	
	func isTimedEvent() -> Bool {
		return (self.trigger.timeBase != nil) && (self.trigger.mins != nil)
	}
	 
	func isAppEvent() -> Bool {
		return (self.trigger.event != nil)
	}
 
	enum eventType: Int {
		case unknown = 0
		case timed
		case event
	}
	
	enum appEventTrigger:  String, CaseIterable {
		case invalid = ""
		case startup	= "startup"
		case shutdown 	= "shutdown"
		
		func description() -> String {
			var str = "Invalid"
	
			switch self {
			case .startup:			str = "Startup"
			case .shutdown:		str = "Shutdown"
				default:
				break
			}
			return str
		}
	}
 
	enum timedEventTimeBase: Int,CaseIterable {
		case invalid = 0
		case midnight
		case sunrise
		case sunset
		case civilSunrise
		case civilSunset
		
		func description() -> String {
			var str = "Invalid"
			
			switch self {
			case .midnight:		str = "Midnight"
			case .sunrise:  		str = "SunRise"
			case .sunset:  		str = "SunSet"
			case .civilSunrise:  str = "Civil SunRise"
			case .civilSunset:  	str = "Civil SunSet"
			default:
				break
			}
			return str
		}
		
		func image() -> UIImage {
			var image:UIImage? =  nil
			
			switch self {
			case .midnight:		image = UIImage(systemName: "clock")
			case .sunrise:  		image = UIImage(systemName: "sunrise")
			case .sunset: 		 	image = UIImage(systemName: "sunset")
			case .civilSunrise:  image = UIImage(systemName: "sunrise.fill")
			case .civilSunset:  	image = UIImage(systemName: "sunset.fill")
			default:					image =   UIImage(systemName: "questionmark")
			}
			return image ?? UIImage()
		}
	}
	
	func eventType() ->  eventType {
		
		if(self.isTimedEvent()) {
			return .timed
		}
		else 	if(self.isAppEvent()) {
			return .event
		}
		return .unknown
	}
	
		enum actionCmd: String, CaseIterable {
		case invalid = ""
		case open	= "open"
		case close	= "close"
		case on		= "on"
		case off		= "off"
 
		func description() -> String {
			var str = "Invalid"
			
			switch self {
			case .on:			str = "On"
			case .off:  		str = "Off"
			case .open:  		str = "Open"
			case .close:  		str = "Close"
			default:
				break
			}
			return str
		}
	}
	
	func allCasesForDeviceID(devID :actionDeviceID) -> [actionCmd]{
		var cmds : [actionCmd] = []
		
		switch devID {
		case .light,
			  .aux:
			cmds = [.on, .off]

		case .door:
			cmds = [.open, .close]

		default:break
		}
		
		return cmds
	}

	func normalizedEvent() -> RESTEvent {
		
		var event = self
		
		switch (event.deviceIDValue()) {
		case .light,
				.aux:
			switch event.cmdValue() {
			case .on, .off:
				break  // these are OK
			case .close:
				event.action.cmd = "off"
			case .open:
				event.action.cmd = "on"
			default:
				event.action.cmd = ""
			}
			
		case .door:
			switch event.cmdValue() {
			case .open, .close:
				break  // these are OK
			case .on:
				event.action.cmd = "open"
			case .off:
				event.action.cmd = "close"
			default:
				event.action.cmd = ""
			}
			
		default: break
			
		}
		return event
	}
	
	func cmdValue() -> actionCmd{
		switch (action.cmd?.lowercased()){
		case "open": return .open
		case "close": return .close
		case "on": return .on
		case "off": return .off
		default: return .invalid
		}
	}
	
	func stringForActionCmd() ->  String {
		let cmd = self.cmdValue()
		let str = cmd.description()
		return str
	}

	enum actionDeviceID: String, CaseIterable {
	case invalid = ""
	case door	= "door"
	case light	= "light"
	case aux	= "aux"

	func description() -> String {
		var str = "Invalid"
		
		switch self {
		case .door:			str = "Coop Door"
		case .light:  		str = "Coop Light"
		case .aux:  		str = "Aux Relay"
 		default:
			break
		}
		return str
	}
}

	func deviceIDValue() -> actionDeviceID{
		switch (action.deviceID?.lowercased()){
		case "door": return .door
		case "light": return .light
		case "aux": return .aux
		default: return .invalid
		}
	}
	
	func stringForActionDeviceID() ->  String {
		let cmd = self.deviceIDValue()
		let str = cmd.description()
		return str
	}

	func stringForTrigger() ->  String {
		
		var str = "Invalid"
		
		switch self.eventType() {
		case .timed:
			if let timebase = self.trigger.timeBase,
				let timedTrigger = timedEventTimeBase(rawValue: timebase)   {
				str = timedTrigger.description()
			}
			
		case .event:
			if self.trigger.event == "startup" {
				str = "Startup"
			}
			else if self.trigger.event == "shutdown" {
				str = "Shutdown"
			}

			break
			
		default:
			break
		}
		
		return str
	}
	
	
	
	func imageForTrigger() -> UIImage {
		
		var image = UIImage(systemName: "questionmark")
	
		switch self.eventType() {
		case .timed:
			if let timebase = self.trigger.timeBase,
				let timedTrigger = timedEventTimeBase(rawValue: timebase)   {
				image = timedTrigger.image()
			}

		case .event:
			if self.trigger.event == "startup" {
				image = UIImage(systemName: "power")
			}
			else if self.trigger.event == "shutdown" {
				image = UIImage(systemName: "poweroff")
			}

			break
			
		default:
			break
		}

		return image ??  UIImage()
	}
}


struct RESTEventTime: Decodable {
	var eventID: String
	var minsFromMidnight: Int
	//var wrappedValue: (String, Int)
	
	//	func encode(to encoder: Encoder) throws {
	//		var unkeyedContainer = encoder.unkeyedContainer()
	//
	//	}
	
	init(from decoder: Decoder) throws {
		
		eventID = ""
		minsFromMidnight = 0
		
		var container = try decoder.unkeyedContainer()
		if (container.count == 2) {
			eventID = try container.decodeIfPresent(String.self)!
			minsFromMidnight = try container.decodeIfPresent(Int.self)!
		}
	}
}
 
struct RESTEventList: Decodable {
	var eventIDs: 			Dictionary<String, RESTEvent>
	var events_timed: 	[RESTEventTime]?
	var future: 			[String]?
	
	enum CodingKeys: String, CodingKey {
		case eventIDs = "eventIDs"
		case events_timed = "events.timed"
		case future = "events.future"
	}
	
//	init(from decoder: Decoder) throws {
//
//		let container = try decoder.container(keyedBy: CodingKeys.self)
//		eventIDs = try container.decode([String: RESTEvent].self, forKey: CodingKeys.eventIDs)
//		future =  try? container.decode([String].self, forKey: CodingKeys.eventIDs)
//
//		let arr = try container.decode(Array<Any>.self, forKey:  CodingKeys.events_timed)
//
//		events_timed = []
////		events_timed  = try? container.decode([RESTEventTime].self, forKey: CodingKeys.events_timed)
//
//	}
}



struct RESTDeviceDoor: Codable {
	var door: DoorState.RawValue

	enum CodingKeys: String, CodingKey {
		case door = "door"
 	}
}

struct RESTDeviceLight: Codable {
	var light: Bool

	enum CodingKeys: String, CodingKey {
		case light = "light"
	}
}

struct RESTDeviceAux: Codable {
	var aux: Bool

	enum CodingKeys: String, CodingKey {
		case aux = "aux"
	}
}


enum BatteryStatus: UInt8 {
	case normal = 0
	case charging_from_in
	case charging_from_5v
	case not_present
	case unknown
}


extension BatteryStatus: Codable {
	
	enum Key: CodingKey {
		case rawValue
	}
	
	enum CodingError: Error {
		case unknownValue
	}
	
	init(from decoder: Decoder) throws {
		let container = try decoder.container(keyedBy: Key.self)
		let rawValue = try container.decode(Int.self, forKey: .rawValue)
		switch rawValue {
		case 0:
			self = .normal
		case 1:
			self = .charging_from_in
		case 2:
			self = .charging_from_5v
		case 3:
			self = .not_present
		default:
			self = .unknown
		}
	}
	
	func encode(to encoder: Encoder) throws {
		var container = encoder.container(keyedBy: Key.self)
		switch self {
		case .normal:
			try container.encode(0, forKey: .rawValue)
		case .charging_from_in:
			try container.encode(1, forKey: .rawValue)
		case .charging_from_5v:
			try container.encode(2, forKey: .rawValue)
		case .not_present:
			try container.encode(3, forKey: .rawValue)
		case .unknown:
			break;
		}
	}
}

enum PowerInputStatus: UInt8 {
	case not_present = 0
	case weak
	case bad
	case present
	case unknown
}

extension PowerInputStatus: Codable {
	
	enum Key: CodingKey {
		case rawValue
	}
	
	enum CodingError: Error {
		case unknownValue
	}
	
	init(from decoder: Decoder) throws {
		let container = try decoder.container(keyedBy: Key.self)
		let rawValue = try container.decode(Int.self, forKey: .rawValue)
		switch rawValue {
		case 0:
			self = .not_present
		case 1:
			self = .weak
		case 2:
			self = .bad
		case 3:
			self = .present
		default:
			self = .unknown
		}
	}
	
	func encode(to encoder: Encoder) throws {
		var container = encoder.container(keyedBy: Key.self)
		switch self {
		case .not_present:
			try container.encode(0, forKey: .rawValue)
		case .weak:
			try container.encode(1, forKey: .rawValue)
		case .bad:
			try container.encode(2, forKey: .rawValue)
		case .present:
			try container.encode(3, forKey: .rawValue)
		case .unknown:
			break;
		}
	}
 
}

func piJuiceImageForStatus(_ status :UInt8?) -> UIImage {
	
	var image = UIImage(systemName: "questionmark")
	
	if let statusByte = status {
		let bat_stat = BatteryStatus(rawValue:(statusByte >> 2) & 0x03);
		let pwr_in_stat = PowerInputStatus(rawValue:(statusByte >> 4) & 0x03);
		let pwr_5v_stat = PowerInputStatus(rawValue:(statusByte >> 6) & 0x03);
		
		if(bat_stat == .charging_from_5v){
			image = UIImage(named: "Battery_charge")
		} else if(bat_stat == .charging_from_in){
			image = UIImage(named: "Battery_charge")
		} else if(bat_stat == .normal){
 			if(pwr_5v_stat == .present) {
				image = UIImage(named: "Battery_charge")		}
			else if(pwr_in_stat == .present) {
				image = UIImage(named: "Battery_charge")			}
			else {
				image = UIImage(named: "Battery")
			}
		} else if(bat_stat == .none){
			image = UIImage(systemName: "bolt.slash.circle.fill")
		}
	}
	
	return image ??  UIImage()
}

func piJuiceTextForStatus(_ status :UInt8?) -> String {

	var str = "Unknown"
 	
	if let statusByte = status {
		let bat_stat = BatteryStatus(rawValue:(statusByte >> 2) & 0x03);
		let pwr_in_stat = PowerInputStatus(rawValue:(statusByte >> 4) & 0x03);
		let pwr_5v_stat = PowerInputStatus(rawValue:(statusByte >> 6) & 0x03);
		
		if(bat_stat == .charging_from_5v){
			str = "Charging"
		} else if(bat_stat == .charging_from_in){
			str = "Charging"
		} else if(bat_stat == .normal){
			if(pwr_5v_stat == .present) {
				str = "Charged"
			}
			else if(pwr_in_stat == .present) {
				str = "Charged"
			}
			else {
				str = "Battery"
			}
	
		} else if(bat_stat == .none){
			str = "No Battery"
		}
	}
	return str
}
 
struct RESTDevices: Codable {
	var light: Bool
	var aux: Bool
	var door: DoorState.RawValue
	var coopTemp: Double
	
	var SOC: Double?
	var pijuice_fault: UInt8?
	var pijuice_status: UInt8?
	var pijuice_pin1: Bool
	var pijuice_pin2: Bool

	enum CodingKeys: String, CodingKey {
		case light = "light"
		case aux = "aux"
		case door = "door"
		case coopTemp = "coopTemp"
		case SOC = "SOC"
		case pijuice_fault = "pijuice.fault"
		case pijuice_status = "pijuice.status"
		case pijuice_pin1 = "pijuice.pin1"
		case pijuice_pin2 = "pijuice.pin2"
	}
}

struct RESTDevicePower: Codable {
	var current_out: Double?
	var voltage_in: Double?
	var voltage_out: Double?
	var tempc: Double?
	var powermode: Bool

	enum CodingKeys: String, CodingKey {
		case current_out = "current.out"
		case voltage_in = "voltage.in"
		case voltage_out = "voltage.out"
		case tempc = "tempc"
		case powermode = "powermode"
	}
}


class CCServerManager: ObservableObject {

	
	enum ServerError: LocalizedError {
		case connectFailed
		case invalidState
		case invalidURL
		case invalidData(String?)
		case unknown
		
		var errorDescription: String? {
			switch self {
			
			case let .invalidData(message):
				return message
				
			default:
				return("this is some bullshit")
				
			}
		}
		
	}
 
	@Published var lastUpdate = Date()
	
	static let shared: CCServerManager = {
			let instance = CCServerManager()
			// Setup code
			return instance
		}()
	
	init () {
		
	}
	
	func executeEvent(_ eventID: String,
						  completion: @escaping (Error?) -> Void)  {
		
		let urlPath = "events/run.actions/\(eventID)"
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			var request = URLRequest(url: urlComps.url!)
			
			
			// Specify HTTP Method to use
			request.httpMethod = "PUT"
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			// Send HTTP Request
			request.timeoutInterval = 30
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				completion(urlError	)
			}
			task.resume()
		}
		else {
			completion(ServerError.invalidURL)
		}
		
	}
	
	func deleteHistoryForValue(_ value: String,
						  completion: @escaping (Error?) -> Void)  {
	
		let urlPath = "valuehistory/\(value)"
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			var request = URLRequest(url: urlComps.url!)
			
			
			// Specify HTTP Method to use
			request.httpMethod = "DELETE"
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			// Send HTTP Request
			request.timeoutInterval = 30
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				completion(urlError	)
			}
			task.resume()
		}
		else {
			completion(ServerError.invalidURL)
		}	}

	func removeEvent(_ eventID: String,
						  completion: @escaping (Error?) -> Void)  {
		
		let urlPath = "events/\(eventID)"
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			var request = URLRequest(url: urlComps.url!)
			
			
			// Specify HTTP Method to use
			request.httpMethod = "DELETE"
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			// Send HTTP Request
			request.timeoutInterval = 30
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				completion(urlError	)
			}
			task.resume()
		}
		else {
			completion(ServerError.invalidURL)
		}
	}

	func createEvent(_ event: RESTEvent,
						  completion:  @escaping (Error?, _ eventID: String?) -> Void)  {

		let urlPath = "events"
		
		if(!event.isValid()){
			completion(ServerError.invalidData("event is not valid"), nil)
		}
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			//			if let queries = queries {
			//				urlComps.queryItems = queries
			//			}
			var request = URLRequest(url: urlComps.url!)
			
			let jsonData = try? event.JSON()
			request.httpBody = jsonData
	
			// Specify HTTP Method to use
			request.httpMethod = "POST"
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			// Send HTTP Request
			request.timeoutInterval = 10
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				if urlError != nil {
					completion(urlError	, nil	)
					return
				}
	
				if let data = data as Data? {
					
					let decoder = JSONDecoder()
					
					if let restErr = try? decoder.decode(RESTError.self, from: data){
	 					completion(ServerError.invalidData(restErr.error.message), nil)
						return
 					}
					else if let obj = try? decoder.decode(RESTEventCreation.self, from: data){
						completion(nil, obj.eventID)
						return
					}
					//
				}
				completion(ServerError.unknown, nil)

			}
			task.resume()
		}
			else {
				completion(ServerError.invalidURL,nil)
			}
	}

	func updateEvent(_ event: RESTEvent,
						  completion:  @escaping (Error?) -> Void)  {
		
		guard  let eventID = event.eventID  else {
			completion(ServerError.invalidData("eventID is not specifed"))
			return
		}
		
		if(!event.isValid() ){
			completion(ServerError.invalidData("event is not valid"))
		}
		
		let urlPath = "events/\(eventID)"
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			//			if let queries = queries {
			//				urlComps.queryItems = queries
			//			}
			var request = URLRequest(url: urlComps.url!)
			
			let jsonData = try? event.JSON()
			request.httpBody = jsonData
			
			// Specify HTTP Method to use
			request.httpMethod = "PATCH"
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			// Send HTTP Request
			request.timeoutInterval = 10
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				if urlError != nil {
					completion(urlError)
					return
				}
				
				if let data = data as Data? {
					
					let decoder = JSONDecoder()
					
					if let restErr = try? decoder.decode(RESTError.self, from: data){
						completion(ServerError.invalidData(restErr.error.message))
					}
					else {
						completion(nil)
					}
					return
					
					//
				}
				completion(ServerError.unknown)
				
			}
			task.resume()
		}
		else {
			completion(ServerError.invalidURL)
		}
	}
 
	func setAux(_ isOn: Bool,
					  completion: @escaping (Error?) -> Void)  {
		
			let urlPath = "devices/aux"
			
			if let requestUrl: URL = AppData.serverInfo.url ,
				let apiKey = AppData.serverInfo.apiKey,
				let apiSecret = AppData.serverInfo.apiSecret {
				let unixtime = String(Int(Date().timeIntervalSince1970))
				
				let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
				//			if let queries = queries {
				//				urlComps.queryItems = queries
				//			}
				var request = URLRequest(url: urlComps.url!)
				
				
				let json = ["state":isOn]
				let jsonData = try? JSONSerialization.data(withJSONObject: json)
				request.httpBody = jsonData
		
				// Specify HTTP Method to use
				request.httpMethod = "PUT"
				request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
				request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
				let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
				request.setValue(sig,forHTTPHeaderField: "Authorization")
				
				// Send HTTP Request
				request.timeoutInterval = 10
				
				let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
				
				let task = session.dataTask(with: request) { (data, response, urlError) in
					
					completion(urlError	)
				}
				task.resume()
			}
				else {
					completion(ServerError.invalidURL)
				}
	}
	
	
	func setLight(_ isOn: Bool,  
					  completion: @escaping (Error?) -> Void)  {
		
			let urlPath = "devices/light"
			
			if let requestUrl: URL = AppData.serverInfo.url ,
				let apiKey = AppData.serverInfo.apiKey,
				let apiSecret = AppData.serverInfo.apiSecret {
				let unixtime = String(Int(Date().timeIntervalSince1970))
				
				let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
				//			if let queries = queries {
				//				urlComps.queryItems = queries
				//			}
				var request = URLRequest(url: urlComps.url!)
				
				
				let json = ["state":isOn]
				let jsonData = try? JSONSerialization.data(withJSONObject: json)
				request.httpBody = jsonData
		
				// Specify HTTP Method to use
				request.httpMethod = "PUT"
				request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
				request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
				let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
				request.setValue(sig,forHTTPHeaderField: "Authorization")
				
				// Send HTTP Request
				request.timeoutInterval = 10
				
				let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
				
				let task = session.dataTask(with: request) { (data, response, urlError) in
					
					completion(urlError	)
				}
				task.resume()
			}
				else {
					completion(ServerError.invalidURL)
				}
	}
	
	func setDoor(_ shouldOpen: Bool,
					  completion: @escaping (Error?) -> Void)  {
		
//		print(String(format:"setDoor %@",  shouldOpen ? "Open" : "Close"))

			let urlPath = "devices/door"
			
			if let requestUrl: URL = AppData.serverInfo.url ,
				let apiKey = AppData.serverInfo.apiKey,
				let apiSecret = AppData.serverInfo.apiSecret {
				let unixtime = String(Int(Date().timeIntervalSince1970))
				
				let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
				//			if let queries = queries {
				//				urlComps.queryItems = queries
				//			}
				var request = URLRequest(url: urlComps.url!)
				
				
				let json = ["state":shouldOpen]
				let jsonData = try? JSONSerialization.data(withJSONObject: json)
				request.httpBody = jsonData
		
				// Specify HTTP Method to use
				request.httpMethod = "PUT"
				request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
				request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
				let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
				request.setValue(sig,forHTTPHeaderField: "Authorization")
				
				// Send HTTP Request
				request.timeoutInterval = 10
	
 	//	print(request)

				let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
				
				let task = session.dataTask(with: request) { (data, response, urlError) in
					
// 		print ( String(decoding: (data!), as: UTF8.self))

					completion(urlError	)
				}
				task.resume()
			}
				else {
					completion(ServerError.invalidURL)
				}
	}
	
	
	func calculateSignature(forRequest: URLRequest, apiSecret: String ) -> String {
		
		if let method: String =  forRequest.httpMethod,
			let urlPath = forRequest.url?.path ,
			let daytimeHeader = forRequest.value(forHTTPHeaderField: "X-auth-date"),
			let apiKey = forRequest.value(forHTTPHeaderField: "X-auth-key")
		{
			var bodyHash:String = ""
			
			if let body = forRequest.httpBody {
				bodyHash = body.sha256String()
			}
			else {
				bodyHash = Data().sha256String()
			}
			
			let stringToSign =   method +  "|" + urlPath +  "|"
				+  bodyHash +  "|" + daytimeHeader + "|" + apiKey
  
			let signatureString = stringToSign.hmac(key: apiSecret)
			
				return signatureString;
		}
	
		return "";

	}
	
	func RESTCall(urlPath: String,
					  httpMethod: String? = "GET",
					  headers: [String : String]? = nil,
					  queries: [URLQueryItem]? = nil,
					  body: Any? = nil,
					  timeout:TimeInterval = 10,
					  completion: @escaping (URLResponse?,  Any?, Error?) -> Void)  {
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			if let queries = queries {
				urlComps.queryItems = queries
			}
			var request = URLRequest(url: urlComps.url!)
			
			// Specify HTTP Method to use
			request.httpMethod = httpMethod
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
		
			if let body = body {
				let jsonData = try? JSONSerialization.data(withJSONObject: body)
				request.httpBody = jsonData
			}
 
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			headers?.forEach{
				request.setValue($1, forHTTPHeaderField: $0)
			}
				
			// Send HTTP Request
			request.timeoutInterval = timeout
			
//  	 	print(request)
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				if urlError != nil {
					completion(nil, nil, urlError	)
					return
				}
			
			
//  			print ( String(decoding: (data!), as: UTF8.self))

				if let data = data as Data? {
					
					let decoder = JSONDecoder()
					
					if let restErr = try? decoder.decode(RESTError.self, from: data){
						completion(response, restErr.error, nil)
					}
					else if let obj = try? decoder.decode(RESTValuesList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTHistory.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTStatus.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDateInfo.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTSchemaList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTProperties.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTEventList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTEvent.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDevices.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDeviceDoor.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDeviceLight.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDeviceAux.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTDevicePower.self, from: data){
						completion(response, obj, nil)
					}
					else if let jsonObj = try? JSONSerialization.jsonObject(with: data, options: .allowFragments) as? Dictionary<String, Any> {
						completion(response, jsonObj, nil)
					}
					else {
						completion(response, nil, nil)
					}
				}
			}
			
			task.resume()
		}
		else {
			completion(nil,nil, ServerError.invalidURL)
		}
	}
}

