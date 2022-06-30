pub mod header{

    pub struct TcpPacketHeader{
        pub  sequence_number: u32,
        pub  ack_number: u32,
        pub  data_offset: usize,
        pub  reserved: usize,
        pub  control_bits: usize,
        pub  window: u16,
        pub  urgent_ptr: u16,
        pub  checksum: u16,
        pub  options: Option<u8>,
        pub  padding: u8, //composed of zeros
        pub  data: u64,
}

impl std::fmt::Display for TcpPacketHeader{
  fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
      write!(f, "
         1. Sequence number {},
         2. Acknowledgement number {},
         3. Data offset {},
         4. Reserved {},
         5. Control bits{},
         6. Window {},
         6. Urgent pointer {},
         8. Checksum {},
         9. Options {:?},
         10. Padding {},
         11. Payload {}",
          self.sequence_number,
          self.ack_number,
          self.data_offset,
          self.reserved,
          self.control_bits,
          self.window,
          self.urgent_ptr,
          self.checksum,
          self.options,
          self.padding,
          self.data,)

  }
  }

}

