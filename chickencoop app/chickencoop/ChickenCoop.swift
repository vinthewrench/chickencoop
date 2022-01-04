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

struct CoopValues {
	
	var door: DoorState
	var light: Bool

	// Tempurature Values
	var temp1: Double
	var cpuTemp: Double
	
	init(){
		self.temp1 = 0
 		self.cpuTemp = 0
		self.door = .unknown
		self.light = false
	}
}

public class ChickenCoop {
	
	var isValid: Bool = false;
	
	static let shared: ChickenCoop = {
		let instance = ChickenCoop()
		// Setup code
		return instance
	}()
	
	
		
	func fetchValues(completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
			fetchData(.values) { result in
				
				if case .success(let v as RESTValuesList) = result {
					
					var ccv :CoopValues = .init()
	 
					if let s1 = v.values["CPU_TEMP"]?.value,
						let val = Double(s1) {
						ccv.cpuTemp = val
						
					}
					if let s1 = v.values["TEMP_0x48"]?.value,
						let val = Double(s1) {
						ccv.temp1 = val
					}
					
					if let s1 = v.values["DOOR_STATE"]?.value,
						let val = DoorState(rawValue: Int(s1)!) {
						ccv.door = val
					}
					
					if let s1 = v.values["LIGHT_STATE"]?.value {
 						ccv.light = s1.boolValue
					}
					completionHandler(.success(ccv))
				}
				else {
					completionHandler(.failure(CoopError.connectFailed))
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

	
//
//	func groupValueKeys(_ keys: [String]) -> [keyGroupEntry]{
//
//		var result:[keyGroupEntry] = []
//
//		result.append( keyGroupEntry(title: "Inverter" ))
//		result.append( keyGroupEntry(title: "SmartShunt" ))
//		result.append( keyGroupEntry(title: "Pump" ))
//		result.append( keyGroupEntry(title: "Temperature" ))
//
//		let pumpKeys = ["TANK", "TANK_RAW", "PUMP_SENSOR"]
//
//		let tempKeys = ["TEMP_0x48","TEMP_0x49","CPU_TEMP"]
//
//		let shuntKeys = [ "AR", "Alarm", "CE", "DM", "FW", "H1", "H10", "H11", "H12", "H15", "H16", "H17", "H18", "H2", "H3", "H4", "H5", "H6", "H7", "H8", "H9", "I", "MON", "P", "PID", "SOC", "TTG", "V", "VM"]
//
//		let inverterKeys = [ "I_BT",  "I_BV",  "I_FREQ",  "I_IPFV",  "I_IPV",  "I_OPC",  "I_OPV",  "I_STATUS"]
//
//		let keys = Array(keys).sorted(by: <)
//
//		for key in keys {
//
//			if(inverterKeys.contains(key)){
//				result[0].keys.append(key)
//			}else if(shuntKeys.contains(key)){
//				result[1].keys.append(key)
//			}else if(pumpKeys.contains(key)){
//				result[2].keys.append(key)
//			}else if(tempKeys.contains(key)){
//				result[3].keys.append(key)
//			}
//		}
//
//		return result
//	}
	
	
	func setLight(_ isOn: Bool,
					  completion:  @escaping (Bool) -> Void = {_ in }) {
		
		CCServerManager.shared.setLight(isOn)
		{ (error)  in
			
			if(error == nil){
				completion(true)
			}
			else {
				completion(false)
			}
			
		}
	}
	func setDoor(_ shouldOpen: Bool,
					  completion:  @escaping (Bool) -> Void = {_ in }) {
		
		CCServerManager.shared.setDoor(shouldOpen)
		{ (error)  in
			
			if(error == nil){
				completion(true)
			}
			else {
				completion(false)
			}
			
		}
	}

}



