use clap::Subcommand;

pub mod completions;
pub mod list;
pub mod mount;
pub mod subvolume;

pub use completions::completions;
pub use list::list;
pub use mount::mount;
pub use subvolume::subvolume;

#[derive(clap::Parser, Debug)]
#[command(name = "bcachefs")]
pub struct Cli {
    #[command(subcommand)]
    subcommands: Subcommands,
}

#[derive(Subcommand, Debug)]
enum Subcommands {
    List(list::Cli),
    Mount(mount::Cli),
    Completions(completions::Cli),
    #[command(visible_aliases = ["subvol"])]
    Subvolume(subvolume::Cli),
}
