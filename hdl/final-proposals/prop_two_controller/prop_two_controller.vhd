library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity prop_two_controller is
	port(
		clk : in std_logic;
		rst : in std_logic;
		avs_read : in std_logic;
		avs_write : in std_logic;
		avs_address : in std_logic_vector(1 downto 0);
		avs_readdata : out std_logic_vector(31 downto 0);
		avs_writedata : in std_logic_vector(31 downto 0);
		
		led : out std_logic;
		button : in std_logic;
		switches : in std_logic_vector(3 downto 0)
	);
end entity;

architecture prop_two_controller_arch of prop_two_controller is
	
	signal switch_register : std_logic_vector(3 downto 0);
	signal button_register, led_register : std_logic;
	
begin
	led <= led_register;
	button_register <= button;
	switch_register <= switches;
		
	avalon_register_read: process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00" =>
					avs_readdata <= (others => '0');
					avs_readdata(0) <= led_register;
				when "01" =>
					avs_readdata <= (others => '0');
					avs_readdata(0) <= button_register;
				when "10" =>
					avs_readdata <= (others => '0');
					avs_readdata(3 downto 0) <= switch_register;
				when others => avs_readdata <= (others => '0');
			end case;
		end if;
	end process;
	
	avalon_register_write: process(clk, rst)
	begin
		if rst = '1' then
			led_register <= '0';
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "00" => led_register <= avs_writedata(0);
				when others => null;
			end case;
		end if;
	end process;
end architecture;