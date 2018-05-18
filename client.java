import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Console;
import java.net.HttpURLConnection;
import java.net.URL;


public class client
{
	public String get(String requestURL)
	{
		try
		{
			URL request = new URL(requestURL);
			HttpURLConnection con = (HttpURLConnection) request.openConnection();
			con.setRequestMethod("GET");		

			BufferedReader br = new BufferedReader(new InputStreamReader((con.getInputStream())));
	
			String output;		
			output = br.readLine();				
		
			con.disconnect();
			return output;
		}
		catch(Exception ex)
		{
				System.out.println("Errored");
		}
		return new String();
	}

	public String delete(String requestURL)
	{
		try
		{
			URL request = new URL(requestURL);
			HttpURLConnection con = (HttpURLConnection) request.openConnection();
			con.setRequestMethod("DELETE");		

			BufferedReader br = new BufferedReader(new InputStreamReader((con.getInputStream())));
	
			String output;		
			output = br.readLine();				
		
			con.disconnect();
			return output;
		}
		catch(Exception ex)
		{
				System.out.println("Errored");
		}
		return new String();
	}

	public String put(String requestURL, String data)
	{
		try
		{
			URL request = new URL(requestURL);
			HttpURLConnection con = (HttpURLConnection) request.openConnection();
			con.setRequestMethod("PUT");								
			con.setDoOutput(true);

			OutputStream os = con.getOutputStream();
			os.write(data.getBytes());
			os.flush();


			BufferedReader br = new BufferedReader(new InputStreamReader((con.getInputStream())));
	
			String output;		
			output = br.readLine();				
		
			con.disconnect();
			return output;
		}
		catch(Exception ex)
		{
				System.out.println("Errored");
				ex.printStackTrace();
				System.out.println(ex);
		}
		return new String();
	}
	public static void main(String[] args)
	{
		client rc=new client();
		String choice;
		String key;
		String value;
		String result;
		String requestbody;
		String data="{\"key\":\"%s\",\"value\":\"%s\"}";
		boolean done=false;
		string address="http://<put ip address here>:6080/"
		do
		{
		System.out.println("Your Options 1)GET 2)PUT 3)DELETE 4)EXIT - Choose 1/2/3/4");
		choice=System.console().readLine();
		switch (choice)
		{
			case "1":
				System.out.println("Enter the Key to retrieve ");
				key=System.console().readLine();
				result = rc.get(address+key);
				System.out.println(result);		
				break;	
			case "2":
				System.out.println("Enter the Key  ");
				key=System.console().readLine();
				System.out.println("Enter the Value ");
				value=System.console().readLine();
				requestbody = String.format(data,key,value);
				result = rc.put(address, requestbody);
				System.out.println(result);
				break;
			case "3":
				System.out.println("Enter the Key to delete ");
				key=System.console().readLine();
				result = rc.delete(address+key);
				System.out.println(result);
				break;

			case "4":
				done=true;
				break;
			default:
				done=true;
		}
		}while (!done);			
	}
}
