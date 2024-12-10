library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity prop_one_controller is
	port(
		clk : in std_logic;
		rst : in std_logic;
		avs_read : in std_logic;
		avs_write : in std_logic;
		avs_address : in std_logic_vector(1 downto 0);
		avs_readdata : out std_logic_vector(31 downto 0);
		avs_writedata : in std_logic_vector(31 downto 0);
		
		leds_out : out std_logic_vector(6 downto 0);
		buttons : in std_logic_vector(1 downto 0)
	);
end entity;

architecture prop_one_controller_arch of prop_one_controller is
	
	signal led_output : std_logic_vector(6 downto 0);
	signal button_1, button_2 : std_logic;
	
begin
	leds_out <= led_output;
	button_1 <= buttons(0);
	button_2 <= buttons(1);
		
	avalon_register_read: process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00" =>
					avs_readdata <= (others => '0');
					avs_readdata(0) <= button_1;
				when "01" =>
					avs_readdata <= (others => '0');
					avs_readdata(0) <= button_2;
				when "10" =>
					avs_readdata <= (others => '0');
					avs_readdata(6 downto 0) <= led_output;
				when others => avs_readdata <= (others => '0');
			end case;
		end if;
	end process;
	
	avalon_register_write: process(clk, rst)
	begin
		if rst = '1' then
			led_output <= "0000000";
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "10" => led_output <= avs_writedata(6 downto 0);
				when others => null;
			end case;
		end if;
	end process;
end architecture;