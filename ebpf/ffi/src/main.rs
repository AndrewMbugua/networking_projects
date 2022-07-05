#[link(name = "badmath", kind = "static")]

extern "C"{
fn bad_add(v1: f32, v2: f32) -> f32;
}


fn main() {
println!("Hello from Rust");

let res = unsafe { bad_add(9., 12.) };
println!("{}?,Is that right?", res);

}
