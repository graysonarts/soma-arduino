fn chan(x: u8) -> u8 {
    (x & 0x0F) % 4
}

fn bank(x: u8) -> u8 {
    (x & 0x0F) / 4
}

fn main() {
    for i in 0..=15 {
        println!(
            "#{:02}: bank({}) chan({})",
            i,
            bank(i | 0x10),
            chan(i | 0x10)
        );
    }
}
