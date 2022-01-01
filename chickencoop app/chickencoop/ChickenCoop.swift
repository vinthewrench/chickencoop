//
//  ChickenCoop.swift
//  chickencoop
//
//  Created by Vincent Moscaritolo on 1/1/22.
//

import Foundation
import CoreLocation

class ServerDateInfo {

//	func getCLLocationCoordinate2D() -> CLLocationCoordinate2D {
//		return  CLLocationCoordinate2DMake(latitude, longitude)
//	}

	let uptime 	: TimeInterval
	
	let latitude : 	CLLocationDegrees?
	let longitude : 	CLLocationDegrees?
	let gmtOffset: 	Int?
	let timeZone: 		String?
	let midNight: 		Date?
	let sunSet:			Date?
	let civilSunSet: 	Date?
	let civilSunRise: Date?
	let sunRise: 		Date?

	init(from :RESTDateInfo){
		
		self.uptime 		= TimeInterval(from.uptime)

		if let latitude = from.latitude ,
			let longitude = from.longitude,
			let midnight = from.midnight,
			let sunSet = from.sunSet,
			let civilSunSet = from.civilSunSet,
			let civilSunRise = from.civilSunRise,
			let sunRise = from.sunRise
			{
	 
			self.latitude 		= latitude
			self.longitude 	= longitude
			self.gmtOffset	 = from.gmtOffset
			self.timeZone	 = from.timeZone

			var t:TimeInterval
			self.midNight 	 = Date(timeIntervalSince1970: midnight)

			t = TimeInterval(midnight + (sunSet * 60))
			self.sunSet 	= Date(timeIntervalSince1970: TimeInterval(t))

			t = TimeInterval(midnight + (civilSunSet * 60))
			self.civilSunSet 	= Date(timeIntervalSince1970: TimeInterval(t))

			t = TimeInterval(midnight + (civilSunRise * 60))
			self.civilSunRise 	= Date(timeIntervalSince1970: TimeInterval(t))

			t = TimeInterval(midnight + (sunRise * 60))
			self.sunRise 	= Date(timeIntervalSince1970: TimeInterval(t))
		}
		else {
			self.latitude = nil
			self.longitude = nil
			self.midNight = nil
			self.sunSet = nil
			self.sunRise = nil
			self.civilSunSet = nil
			self.civilSunRise = nil
			self.gmtOffset = nil
			self.timeZone  = nil
		}
	}
}


 
struct keyGroupEntry {
	var title = String()
	var keys = [String]()
}

class PumpValueFormatter: Formatter {
	
	var units = RESTschemaUnits.UNKNOWN
	
	func unitSuffixForUnits(forUnits unit: RESTschemaUnits) -> String {
		
		var suffix = String()
		
		switch unit {
		case .MILLIVOLTS,
			  .VOLTS:
			suffix = "V"
			break
			
		case .MILLIAMPS, .AMPS:
			suffix = "A"
			break
			
		case .MAH:
			suffix = "Ahrs"
			break
			
		case .DEKAWATTHOUR:
			suffix = "kWh"
			break
			
		case .DEGREES_C:
			suffix = "ºC"
			break
			
		case .PERMILLE,
			  .PERCENT:
			suffix = "%"
			break
			
		case .WATTS:
			suffix = "W"
			break
			
		case .SECONDS:
			suffix = "Seconds"
			break
			
		case .MINUTES:
			suffix = "Minutes"
			break
			
		case .HERTZ:
			suffix = "Hz"
			break
			
		default:
			break
		}
		
		if !suffix.isEmpty {
			suffix = " " + suffix
		}
		
		return suffix
	}
	
	
	func normalizedDoubleForValue(in string: String, forUnits unit: RESTschemaUnits) -> Double {
		var  retVal: Double = 0
		
		if let val = Double(string) {
			
			switch unit {
			case .MILLIVOLTS,
				  .MILLIAMPS,
				  .MAH:
				retVal = val / 1000;
				break
				
			case .DEKAWATTHOUR:
				retVal = val / 100;
				break;
				
			case .PERMILLE:
				retVal = val / 10;
				break;
				
			case .PERCENT,
				  .DEGREES_C,
				  .WATTS,
				  .VOLTS,
				  .AMPS,
				  .SECONDS,
				  .MINUTES,
				  .HERTZ:
				retVal = val;
				
			default:
				break
			}
		}
		return retVal
	}
	
	func normalizedIntForValue(in string: String, forUnits unit: RESTschemaUnits) -> Int {
		var  retVal: Int = 0
			
		switch unit {
 
		case .MINUTES,
			  .SECONDS,
			  .BOOL,
			  .INT:
			if let val = Int(string) {
				retVal = val
			}
			break
			
		case .VE_PRODUCT:
			
			let range = NSRange(location: 0, length: string.utf16.count)
			let regex = try! NSRegularExpression(pattern: "^0?[xX][0-9a-fA-F]{4}$")
			if(regex.firstMatch(in: string, options: [], range: range) != nil){
				if let intVal = Int(string, radix: 16) {
					retVal = intVal
				}
			}
			break;
			
		default:
			break
		}
		
		return retVal
	}
	
	func displayStringForValue(in string: String, forUnits unit: RESTschemaUnits) -> String {
		
		var retVal = string
		let suffix =  self.unitSuffixForUnits(forUnits: units)
		
		switch unit {
		case .MILLIVOLTS,
			  .MILLIAMPS,
			  .MAH,
			  .VOLTS,
			  .AMPS,
			  .HERTZ,
			  .WATTS,
			  .DEKAWATTHOUR,
			  .PERMILLE,
			  .PERCENT:
		
			let  val = normalizedDoubleForValue(in: string, forUnits: units)
			retVal = String(format: "%3.2f%@", val, suffix )
			break

		case .BOOL:
			let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == 0){
				retVal = "No"
			}  else if(val == 1){
				retVal = "Yes"
			}
			break
	
		case .DEGREES_C:
			let  val = normalizedDoubleForValue(in: string, forUnits: units)
			let  tempF =  val * 9.0 / 5.0 + 32.0
			retVal = String(format: "%3.2f%@", tempF, "°F")
			break
			
		case .SECONDS,
			  .MINUTES,
			  .INT:
				let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == -1){
				retVal = "Infinite";
			}
			else
			{
				retVal = String(format: "%d %@", val, suffix)
			}
			break
			
		case .VE_PRODUCT:
				let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == 0xA389){
				retVal = "SmartShunt 500A/50mV";
			}
	
			break
			
		default:
			break
		}
		
		
		return retVal
	}

	
	func replaceValueWithString(in string: String, forUnits unit: RESTschemaUnits) -> String {
		var rawString = string
		
		rawString = displayStringForValue(in: string, forUnits: 	unit)
			
	//		String(format: "%@%@", rawString, self.unitSuffixForUnits(forUnits: units))
		
		return rawString
	}
	
	
	override func string(for obj: Any?) -> String? {
		
		if let string = obj as? String {
			return replaceValueWithString(in: string , forUnits: units)
		}
		return nil
	}
}


enum CoopError: Error {
	case connectFailed
	case invalidState
	case invalidURL
	case restError
	case internalError
	
	case unknown
}

enum CoopRequest: Error {
	case status
	case date
	case event
	case events

	case values
	case props
	case schema
	case unknown
}
 
enum DeviceState: Int {
	case unknown = 0
	case disconnected
	case connected
	case error
	case timeout
}

enum WellPumpState: Int {
	case unknown = 0
	case paused
	case pumping
	case off
}

struct CoopValues {
	
	// Tank Values
	var rawTank: Double
	var tankGals: Double
	var tankEmpty: Double
	var tankFull: Double
	var tankPercent: Double
	
	// inverter / charger values
	var acIn: Double
	var acOut: Double
	var acHz: Double
	var invLoad: Double
	var batVolts: Double
	var batAmps: Double
	var batSOC: Double
	var utilityFail : Bool
	var batteryLow : Bool
 
	var inverterState: DeviceState
	var inverterLastTime : Int
	var batteryState: DeviceState
	var batteryLastTime : Int

	var batteryTime: Double
	var batteryConsumed: Double

	var temp1: Double
	var temp2: Double
	var cpuTemp: Double

	var pumpState : WellPumpState
	var pressureTankState : WellPumpState

	init(){
		self.inverterState = .unknown
		self.batteryState = .unknown
		self.pumpState = .unknown
		self.pressureTankState = .unknown
		self.inverterLastTime = 0
		self.batteryLastTime = 0
		
		self.rawTank = 0
		self.tankGals = 0
		self.tankEmpty = 0
		self.tankFull = 0
		self.tankPercent = 0
		
		self.acIn = 0
		self.acOut = 0
		self.acHz = 0
		self.invLoad = 0
		self.batAmps = 0
		self.batSOC = 0
		self.batVolts = 0
		self.temp1 = 0
		self.temp2 = 0
		self.cpuTemp = 0
		self.batteryTime = 0
		self.batteryConsumed = 0
		self.utilityFail = false
		self.batteryLow = false
	}
}

public class ChickenCoop {
	
	var isValid: Bool = false;
	
	static let shared: ChickenCoop = {
		let instance = ChickenCoop()
		// Setup code
		return instance
	}()
	
	
	struct cachedValues_t {
		var tankGals: Double
		var tankEmpty: Double
		var tankFull: Double
		var isValid: Bool
		
		init() {
			self.tankGals = 0
			self.tankEmpty = 0
			self.tankFull = 0
			self.isValid = false
		}
	}
	
	var cachedValues :cachedValues_t
	
	private init(){
		cachedValues = .init()
	}
	
	
	func fetchValues(completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		// get the list of cached properties first
		if(!cachedValues.isValid){
			
			fetchData(.props) { result in
				
				if case .success(let props as RESTProperties) = result {
					var cnt = 0
					if let s1 = props.properties["prop-tank-empty"] {
						self.cachedValues.tankEmpty = Double(s1) ?? 0.0
						cnt+=1
					}
					if let s1 = props.properties["prop-tank-full"] {
						self.cachedValues.tankFull = Double(s1) ?? 0.0
						cnt+=1
					}
					if let s1 = props.properties["prop-tank-gals"] {
						self.cachedValues.tankGals = Double(s1) ?? 0.0
						cnt+=1
					}
					
					self.cachedValues.isValid = cnt == 3
					
					if(self.cachedValues.isValid) {
						self.fetchValues(completionHandler: completionHandler)
					}
					else {
						completionHandler(.failure(CoopError.internalError))
					}
					
				}
				else {
					completionHandler(.failure(CoopError.connectFailed))
					
				}
			}
		}
		else {
			fetchData(.values) { result in
				
				if case .success(let v as RESTValuesList) = result {
					
					var phv :CoopValues = .init()
					
					phv.inverterState = DeviceState(rawValue:v.inverter) ?? .unknown
					phv.batteryLastTime = v.batteryLastTime
					
					phv.batteryState = DeviceState(rawValue:v.battery) ?? .unknown
					
					if let s1 = v.values["TANK_RAW"]?.value,
						var tank_raw = Double(s1) {
						
						let empty =  self.cachedValues.tankEmpty
						let full =  self.cachedValues.tankFull
						let gals =  self.cachedValues.tankGals
						
						if(tank_raw < empty){ tank_raw = empty}
						if (tank_raw > full) {tank_raw = full}
						let tankP  = ((tank_raw - empty) / (full - empty))
						
						phv.tankEmpty = empty
						phv.tankFull = full
						phv.tankPercent =  tankP * 100.00
						phv.rawTank = tank_raw
						phv.tankGals = gals * tankP
					}
					
					if let s1 = v.values["I_IPFV"]?.value,
						let val = Double(s1) {
						phv.acIn = val
					}
					if let s1 = v.values["I_OPV"]?.value,
						let val = Double(s1) {
						phv.acOut = val
					}
					if let s1 = v.values["I_FREQ"]?.value,
						let val = Double(s1) {
						phv.acHz = val
					}
					if let s1 = v.values["I_OPC"]?.value,
						let val = Double(s1) {
						phv.invLoad = val
					}
					
					if let s1 = v.values["SOC"]?.value,
						let val = Double(s1) {
						phv.batSOC = val
					}
					
					if let s1 = v.values["V"]?.value,
						let val = Double(s1) {
						phv.batVolts = val
					}
					if let s1 = v.values["I"]?.value,
						let val = Double(s1) {
						phv.batAmps = val
					}
					if let s1 = v.values["I_STATUS"]?.value,
						s1.count == 8 {
						phv.utilityFail =  s1[0]=="1"
						phv.batteryLow = s1[1] == "1"
					}
					
					if let s1 = v.values["CPU_TEMP"]?.value,
						let val = Double(s1) {
						phv.cpuTemp = val
						
					}
					if let s1 = v.values["TEMP_0x48"]?.value,
						let val = Double(s1) {
						phv.temp1 = val
					}
					
					if let s1 = v.values["TEMP_0x49"]?.value,
						let val = Double(s1) {
						phv.temp2 = val
					}
					
					if let s1 = v.values["TTG"]?.value,
						let val = Double(s1) {
						phv.batteryTime = val
					}
					
					if let s1 = v.values["CE"]?.value,
						let val = Double(s1) {
						phv.batteryConsumed = val
					}
					
					if let s1 = v.values["PUMP_SENSOR"]?.value,
						s1.count == 8 {
						
						let b2 =  s1[5]=="1"
						let b1 =  s1[6]=="1"
						let b0 =  s1[7]=="1"
						
						phv.pumpState = .unknown
						if(!b0 && !b1) {
							phv.pumpState = .paused
						}
						else if(b0 && !b1) {
							phv.pumpState = .off
						}
						else if(b0 && b1) {
							phv.pumpState = .pumping
						}
						
						if(b2){
							phv.pressureTankState = .pumping
						}
						else {
							phv.pressureTankState = .off
						}
						
					}
					
					completionHandler(.success(phv))
					
				}
				else {
					completionHandler(.failure(CoopError.connectFailed))
				}
				
			}
		}
	}
	
	func fetchData(_ requestType: CoopRequest,  ID : String? = nil,
						completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		var urlPath :String = ""
		
		switch(requestType){
		case .status:
			urlPath = "state"
			
		case .date:
			urlPath = "date"
 
		case .events:
			urlPath = "events"
	
		case .event:
			if let evtID = ID {
				urlPath = "events/" + evtID
			}

		case .values:
			urlPath = "values"
			
		case .props:
			urlPath = "props"
			
		case .schema:
			urlPath = "schema"
			
		default:
			break;
		}
		
		CCServerManager.shared.RESTCall(urlPath: urlPath,
												  headers:nil,
												  queries: nil) { (response, json, error)  in
			
			if (json == nil) {
				completionHandler(.failure(CoopError.connectFailed))
			}
			else 	if let values = json as? RESTValuesList {
				completionHandler(.success(values))
			}
			else 	if let props = json as? RESTProperties {
				completionHandler(.success(props))
			}
			else 	if let status = json as? RESTStatus {
				completionHandler(.success(status))
			}
			else if let obj = json as? RESTDateInfo {
				let date =  ServerDateInfo(from: obj)
				completionHandler(.success(date))
			}
			else 	if let events = json as? RESTEventList {
				completionHandler(.success(events))
			}
			else 	if let event = json as? RESTEvent {
				completionHandler(.success(event))
			}
				else 	if let schema = json as? RESTSchemaList {
				completionHandler(.success(schema))
			}
			else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
		}
	}
	
//	func fetchHistory(_ key: String,
//							completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
//
//		let urlPath :String = "history/" + key
//
//		let headers = ["limit" : "500"]
//
//		CCServerManager.shared.RESTCall(urlPath: urlPath,
//												  headers:headers,
//												  queries: nil) { (response, json, error)  in
//
//			if let restHist = json as? RESTHistory {
//				completionHandler(.success(restHist))
//			}
//			else if let restErr = json as? RESTError {
//				completionHandler(.success(restErr))
//			}
//			else if let error = error{
//				completionHandler(.failure(error))
//			}
//		}
//	}
	
	
	func fetchEvents( completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		let urlPath :String = "events"
		
		CCServerManager.shared.RESTCall(urlPath: urlPath,
												  headers:nil,
												  queries: nil) { (response, json, error)  in
						
			if let restEvents = json as? RESTEventList {
				completionHandler(.success(restEvents))
			}
			else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
			else {
				let error = CoopError.internalError
				completionHandler(.failure(error))
			}
		}
	}

	
	
	func groupValueKeys(_ keys: [String]) -> [keyGroupEntry]{
		
		var result:[keyGroupEntry] = []
		
		result.append( keyGroupEntry(title: "Inverter" ))
		result.append( keyGroupEntry(title: "SmartShunt" ))
		result.append( keyGroupEntry(title: "Pump" ))
		result.append( keyGroupEntry(title: "Temperature" ))
		
		let pumpKeys = ["TANK", "TANK_RAW", "PUMP_SENSOR"]
		
		let tempKeys = ["TEMP_0x48","TEMP_0x49","CPU_TEMP"]
	
		let shuntKeys = [ "AR", "Alarm", "CE", "DM", "FW", "H1", "H10", "H11", "H12", "H15", "H16", "H17", "H18", "H2", "H3", "H4", "H5", "H6", "H7", "H8", "H9", "I", "MON", "P", "PID", "SOC", "TTG", "V", "VM"]
		
		let inverterKeys = [ "I_BT",  "I_BV",  "I_FREQ",  "I_IPFV",  "I_IPV",  "I_OPC",  "I_OPV",  "I_STATUS"]
		
		let keys = Array(keys).sorted(by: <)
		
		for key in keys {
			
			if(inverterKeys.contains(key)){
				result[0].keys.append(key)
			}else if(shuntKeys.contains(key)){
				result[1].keys.append(key)
			}else if(pumpKeys.contains(key)){
				result[2].keys.append(key)
			}else if(tempKeys.contains(key)){
				result[3].keys.append(key)
			}
		}
		
		return result
	}
}

