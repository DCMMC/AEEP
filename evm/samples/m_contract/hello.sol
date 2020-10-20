pragma solidity ^0.4.0; 


contract Hello{
    
    uint256 name;
    string name2;
    
    constructor(uint256 _name, string _name2) public {
        name = _name;
        name2 = _name2;
    }
    
    function getName() public view returns(uint256) {
        return name;
    }
    
    function getName2() public view returns(string) {
        return name2;
    }
    
    function  setName(uint256 _name) public payable returns(bool){
        name = _name;
        return true;
    }
    
    function  setName2(string _name) public payable returns(bool){
        name2 = _name;
        return true;
    }
    
}