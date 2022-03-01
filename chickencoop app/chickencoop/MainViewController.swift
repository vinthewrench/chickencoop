//
//  ViewController.swift
//  chickencoop
//
//  Created by Vincent Moscaritolo on 12/31/21.
//

import UIKit


@objc protocol MainSubviewViewControllerDelegate  {
	func addButtonHit(_ sender: UIButton)
	func shouldShowAddButton() -> Bool 
	func refreshView()
}


class MainSubviewViewController: UIViewController {
	var mainView : MainViewController?
}
 
class MainViewController: UIViewController, UITabBarDelegate, SetupViewControllerDelegate {
	
	
	
	@IBOutlet var tabBar	: UITabBar!
	@IBOutlet var lbTitle 	: UILabel!
	@IBOutlet var vwContainer : UIView!

	@IBOutlet var btnAdd 	: UIButton!

	@IBOutlet var btnDetails 	: UIButton!
	@IBOutlet var btnSetup 	: 	UIButton!

	var containViewController : UIViewController? = nil

	var subViewDelegate: MainSubviewViewControllerDelegate? = nil

	var timer = Timer()

	override var title: String? {
		 get {
			 return lbTitle.text ?? ""
		 }
		 set(name) {
			 lbTitle.text = name
		 }
	}
	
	

	override func viewDidLoad() {
		super.viewDidLoad()
		
		
	//	let index = AppData.serverInfo.tabSelection
	
		 let item = AppData.serverInfo.tabSelection
		if( item < tabBar.items!.count){
			tabBar.selectedItem = tabBar.items![item]
			self.tabBar(tabBar, didSelect: tabBar.selectedItem!)
 		}
 	}

	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
		
		if(AppData.serverInfo.validated){
			refreshWarning();
			startPolling();
		}
		else {
			stopPollng();
		}
	}
	
	override func viewDidAppear(_ animated: Bool) {
		super.viewDidAppear(animated)
		
		if(!AppData.serverInfo.validated){
			self.displaySetupView();
		}
	
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		
		stopPollng();
 	}
	
	// MARK: - polling / refresh

	public func startPolling() {
		
		timer =  Timer.scheduledTimer(withTimeInterval: 5.0,
												repeats: true,
												block: { timer in
													DispatchQueue.main.async  {
														self.refreshWarning()
													}
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}

	
	func tabBar(_ tabBar: UITabBar, didSelect item: UITabBarItem) {
		self.title = item.title
		AppData.serverInfo.tabSelection = item.tag
		
		var newVC:MainSubviewViewController? = nil;
 
		switch(item.tag){
		case 0:
			newVC = CoopViewController.shared
			break;
 
		case 1:
			newVC = SchedulesViewController.shared
			break;
 
		default:
			break;
		}
		
		if(newVC == containViewController){
			return
		}
		
		newVC?.mainView = self
		self.subViewDelegate = newVC as? MainSubviewViewControllerDelegate
		
		btnAdd.isHidden = subViewDelegate == nil
		
		if let cvc = containViewController {
			cvc.willMove(toParent: nil)
			cvc.view.removeFromSuperview()
			cvc.removeFromParent()
			containViewController = nil
		}
	
		if let newVC = newVC {
			containViewController = newVC
			newVC.view.frame = self.vwContainer.bounds
			newVC.willMove(toParent: self)
			self.vwContainer.addSubview(newVC.view)
			self.addChild(newVC)
			newVC.didMove(toParent: self)
		}
	 
	}
	
	private func refreshWarning() {
		
		if(AppData.serverInfo.validated){
			
			let lastEtag = AppData.serverInfo.lastErrorEtag
			
			ChickenCoop.shared.fetchErrorHistory(limit: 0, eTag: lastEtag) { result in
				
				if case .success(let hist as RESTErrorLog ) = result {
					
					var color =  UIView.appearance().tintColor
					
					if let newTag = hist.eTag  {
						if(hist.count == 0){
							AppData.serverInfo.lastErrorEtag =  UInt64(newTag)
						}
						else {
							color = .systemRed
						}
						
					} else {
						AppData.serverInfo.lastErrorEtag = 0
					}
					
					self.btnDetails.tintColor = color
				}
			}
		}
	}
 
	// SetupViewControllerDelegate
	func setupViewChanged(state: loginState){

		if(state == loginState.success){
			refreshWarning();
 			startPolling();
			subViewDelegate?.refreshView()
		}
		else {
			stopPollng();
			subViewDelegate?.refreshView()
		}
	}

	// actions
	@IBAction func SetupUpClicked(_ sender: UIButton) -> Void {

		self.displaySetupView()
	}

	

	func displaySetupView(){

		if let setupView = SetupViewController.createWith(delegate: self) {
			self.show(setupView, sender: self)
		}
	}
	
	@IBAction func addBtnUpClicked(_ sender: UIButton) -> Void {
		subViewDelegate?.addButtonHit(sender)

	}

	@IBAction func DetailsClicked(_ sender: UIButton) -> Void {
		
		if let coopValues = CoopValuesViewController.create() {

			self.show(coopValues, sender: self)
		}
		
	}
	
}
