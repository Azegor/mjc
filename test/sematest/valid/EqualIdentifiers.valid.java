class EqualIdentifiers {

	public int counter;

	public static void main(String[] args) {

		EqualIdentifiers equalIdentifiers = new EqualIdentifiers();
		equalIdentifiers.equalIdentifiers(equalIdentifiers.equalIdentifiers(new EqualIdentifiers()));
	}

	public EqualIdentifiers equalIdentifiers(EqualIdentifiers equalIdentifiers) {
		while (count() < 10) {
			equalIdentifiers.count();
		}
		return equalIdentifiers;
	}

	public int count() {
		counter = counter + 1;
		return counter;
	}
}
