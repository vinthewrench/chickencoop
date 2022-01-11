//
//  CoopViewController.swift
//  chickencoop
//
//  Created by Vincent Moscaritolo on 1/2/22.
//

import UIKit

@IBDesignable
class CircleBackgroundView: UIView {

	 override func layoutSubviews() {
		  super.layoutSubviews()
		  layer.cornerRadius = bounds.size.width / 2
		  layer.masksToBounds = true
	 }

}


// MARK: - CoopViewController

class CoopViewController: MainSubviewViewController {
	
	@IBOutlet var vwOverlay	: UIView!

	@IBOutlet var btnUp	: UIButton!
	@IBOutlet var vwUpCircle	: UIView!

	@IBOutlet var btnDown	: UIButton!
	@IBOutlet var vwDownCircle	: UIView!

	@IBOutlet var swLight	: UISwitch!
	@IBOutlet var lblTemp	: UILabel!
	
	func tempInFahrenheit(_ temperature: Double) -> Double {
		let fahrenheitTemperature = temperature * 9 / 5 + 32
		return fahrenheitTemperature
	}

	var timer = Timer()

	static let shared: CoopViewController! = {
		
		let storyboard = UIStoryboard(name: "CoopView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "CoopViewController") as? CoopViewController
		
		return vc
	}()

// MARK: - view lifetime
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
 	}
	
	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
		 
		mainView?.title = "Chicken Coop"
		
		refreshButtons(.unknown);
		
  		if(AppData.serverInfo.validated){
			startPolling();
		}
		else {
			stopPollng();
		}
		
		refreshView()

	
	}

	override func viewDidAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		
		stopPollng();
//		refreshButtons(.unknown);
	}

	// MARK: - polling / refresh

	public func startPolling() {
		
		timer =  Timer.scheduledTimer(withTimeInterval: 0.8,
												repeats: true,
												block: { timer in
													DispatchQueue.main.async  {
														self.refreshView()
													}
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}

	
	var lastDoorState: DoorState = .unknown

	private func refreshButtons(_ state: DoorState){
		
		if(state != lastDoorState){
			
//			print("refreshButtons: \(state)")
			
			switch(state){
			case .open:
				self.vwUpCircle.alpha = 1;
				self.vwDownCircle.alpha = 0;
				self.vwUpCircle.layer.removeAllAnimations()
				self.vwDownCircle.layer.removeAllAnimations()
				break;
				
			case .closed:
				self.vwUpCircle.layer.removeAllAnimations()
				self.vwDownCircle.layer.removeAllAnimations()
				self.vwUpCircle.alpha = 0;
				self.vwDownCircle.alpha = 1;
				break;
				
			case .opening:
				self.vwDownCircle.alpha = 0;
				self.vwUpCircle.alpha = 0;
				UIView.animate(withDuration: 0.6, //Time duration you want,
									delay: 0.0,
									options: [.curveEaseInOut, .autoreverse, .repeat],
									animations: { [weak self] in self?.vwUpCircle.alpha = 1.0 },
									completion: nil //{ [weak self] _ in self?.vwUpCircle?.alpha = 1.0 }
				)
				break;
				
			case .closing:
				self.vwUpCircle.alpha = 0;
				self.vwDownCircle.alpha = 0;
				UIView.animate(withDuration: 0.6, //Time duration you want,
									delay: 0.0,
									options: [.curveEaseInOut, .autoreverse, .repeat],
									animations: { [weak self] in self?.vwDownCircle.alpha = 1.0 },
									completion: nil //{ [weak self] _ in self?.vwDownCircle?.alpha = 1.0 }
				)
				
				break;
				
			case .unknown:
				self.vwUpCircle.alpha = 0;
				self.vwDownCircle.alpha = 0;
				break;
			}
			
			lastDoorState = state;
		}
	}
	
	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			
			self.vwOverlay.isHidden = true
			
			ChickenCoop.shared.fetchData(.door) { result in
				if case .success(let dev as RESTDeviceDoor) = result {
					if let state = DoorState(rawValue: dev.door) {
						self.refreshButtons(state)
					}
				}
			}
 
			ChickenCoop.shared.fetchData(.light) { result in
				if case .success(let dev as RESTDeviceLight) = result {
					self.swLight.isOn = dev.light
				}
			}
			
			ChickenCoop.shared.fetchValues() { result in
				if case .success(let ccv as CoopValues ) = result {
		 				self.lblTemp.text =  String(format: "%.0f°F", self.tempInFahrenheit(ccv.temp1 ))
				}
			}
		}
		else {
			self.vwOverlay.isHidden = false
			
		}
	}

	// MARK: - actions

	@IBAction func switchChanged(sender: UISwitch) {
		stopPollng()
		DispatchQueue.main.async  {
			ChickenCoop.shared.setLight(sender.isOn) {_ in
				self.startPolling()
			}
		}
	}
		
	
	@IBAction func tapGesture(recognizer : UITapGestureRecognizer) {
		if recognizer.state == .ended{
			
			stopPollng()
			
	 		self.swLight.isOn = !self.swLight.isOn
			DispatchQueue.main.async  {
				ChickenCoop.shared.setLight(self.swLight.isOn) {_ in
					self.startPolling()
				}
			}
		}
	}
	
	@IBAction func btnUpClicked(_ sender: UIButton) -> Void {
		
		stopPollng()
		self.refreshButtons(.opening)

		DispatchQueue.main.async  {
			ChickenCoop.shared.setDoor(true) {_ in
				self.startPolling()
			}
		}
	}
	
	@IBAction func btnDownClicked(_ sender: UIButton) -> Void {
 
		stopPollng()
		self.refreshButtons(.closing)

		DispatchQueue.main.async  {
			ChickenCoop.shared.setDoor(false) {_ in
				self.startPolling()
			}
		}
	}
}
	
