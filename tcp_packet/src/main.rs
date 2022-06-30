#![allow(unused_imports)]
use std::io::Write;
use std::net::Ipv6Addr;
use std::net::TcpStream;
use tcp_packet::header::TcpPacketHeader; //My own public module 

//To be able to convert struct to u8 bytes
unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::std::slice::from_raw_parts(
        (p as *const T) as *const u8,
        ::std::mem::size_of::<T>(),
    )
}

//begin here
fn main(){

let _source = Ipv6Addr::LOCALHOST;
let _destination = Ipv6Addr::new(0, 0, 0, 0, 0, 0, 0, 1);
let tcp_packet = TcpPacketHeader{
                sequence_number: 0,
                ack_number: 0,
                data_offset: 0,
                reserved: 0, //reserved for future use,must be 0
                control_bits: 0,
                window: 0,
                urgent_ptr: 0,
                checksum: 0,
                options: Some(0),
                padding: 0, //composed of zeros
                data: 20, 
};
/*
Left commented in case I need to serializing data to JSON
let serialized_data = serde_json::to_string(&tcp_packet).unwrap();
let bytes_s: &[u8] = serialized_data.as_bytes();
*/

let converted_bytes: &[u8] = unsafe { any_as_u8_slice(&tcp_packet) };
//sending bytes over web sockets
//let mut socket = TcpStream::connect("ip_address:port_number").unwrap();
//socket.write_all(converted_bytes).unwrap();

println!("Coverted bytes:{:?}",converted_bytes);
//utf8 to string
//println!("\n{:?}",String::from_utf8(converted_bytes.to_vec()));




//A converter that decodes a byte stream into Unicode





















}
